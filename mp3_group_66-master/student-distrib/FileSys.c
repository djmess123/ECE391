#include "FileSys.h"
#include "rtc.h"
#include "syscalls.h"
#include "terminal.h"


uint32_t* iNode_Start;          //Address for first index nodes
uint32_t* data_Block_Start;     //Address for first data block
uint32_t* bootBlock_Start;      //Address for start of BootBlock

boot_block_t* bootBlock;        //Pointer to BootBlock;   


/***
 * is_fd_active
 * checks validity of fd
 * INPUT: fd: file descriptor
 * OUTPUT: None
 * RETURN: 1 for active, 0 for not active
 * SIDE EFFECTS: none
 */
int32_t is_fd_active(int32_t fd)
{
    PCB_t* PCB=terminal[cur_term].cur_PCB;
    return PCB->File_Desc_Array[fd].flags & 0x1; //bit 1 is the active flag
}



/***
 * fileSystem_init
 * initializes the file system
 * INPUT: fs_start: memory location for file ssytem from bootloader
 * OUTPUT: None
 * RETURN: 0 for success, -1 for failure
 * SIDE EFFECTS: modifies boot block
 */
int fileSystem_init(uint32_t * fs_start){
    if(fs_start == NULL){                                       //null guard
        return -1;
    }
    //int i;
    bootBlock=(boot_block_t*)fs_start;                          //Cast start address to bootblock
    uint32_t numIndexnodes=bootBlock->stats.numInodes;          //Get Number of Inodes
    bootBlock_Start=(uint32_t*)bootBlock;                       
    iNode_Start=fs_start+BLOCK_SIZE/4;                          //Set pointer to first Inode
    data_Block_Start=fs_start+(numIndexnodes+1)*BLOCK_SIZE/4;   //Set pointer to first DataBlock
    /*for(i=0;i<bootBlock->stats.numDirEntries;i++){
        printf("%s\n",bootBlock->dirEntries[i].name);
    }*/
    return 0;
}

/***
 * read_dentry_by_name
 * grabs directory entry via a name
 * INPUT: fname: string with name of directory entry
 * OUTPUT: dentry: struct filled out with info from file system
 * RETURN: 0 for success, -1 for failure
 * SIDE EFFECTS: modifies dentry arg, calls read_dentry_by_index
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    int i;
    int index=-1;
    if(dentry==NULL||fname==NULL)                                    //NULL check
    {
        return -1;
    }
    for(i=0;i<bootBlock->stats.numDirEntries;i++){      //Iterate through each dentry to find the matching name
        dentry_t cur=(bootBlock->dirEntries[i]);
        if(strncmp((const int8_t*)fname,cur.name,NAME_LENGTH)==0){              //If we have found the matching name set the index
            index=i;
            break;
        }
    }
    if(index==-1){                                      //If there is no matching index, return
        return -1;
    }
    return read_dentry_by_index(index, dentry);        //Use index of matching dentry as input
}


/***
 * read_dentry_by_index
 * grabs directory entry via an index
 * INPUT: index: index of directory entry
 * OUTPUT: dentry: struct filled out with info from file system
 * RETURN: 0 for success, -1 for failure
 * SIDE EFFECTS: modifies dentry arg
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    if(dentry==NULL){                                           //Null Check
        return -1;
    }
    if(index<0||index>=bootBlock->stats.numDirEntries){         //Check if index is out of bounds
        return -1;
    }
    dentry_t source=bootBlock->dirEntries[index];               //Get source dentry
    strcpy(dentry->name,source.name);                           //Deep copy source dentry to destination dentry
    dentry->fileType=source.fileType;
    dentry->inode=source.inode;
    return 0;
}


/***
 * read_data
 * grabs data from file from a location inside file
 * INPUT: inodex: inode index
 *        offset: how many bytes from start of file
 *        length: how many bytes to read
 * OUTPUT: buf: buffer large enough to fit length number of bytes
 * RETURN: number of bytes successfully read, -1 for failure
 * SIDE EFFECTS: modifies buf, calls memcpy
 */
int32_t read_data(uint32_t inodex, uint32_t offset, uint8_t* buf, uint32_t length){
    if (buf == NULL)                                                        //Null Check
        return -1;
    if (inodex < 0 || inodex >= NUM_INODES)                                 //Check if inode is out of bounds
        return -1;
    
    inode_t* curInode=(inode_t*)(iNode_Start+BLOCK_SIZE/4*inodex);          //Get inode at index

    if (length + offset > curInode->blockLength)                            //Clip number of bytes if would be too many
        length = curInode->blockLength - offset;


    uint32_t BlockId=offset/BLOCK_SIZE;
    uint32_t DataId=offset%BLOCK_SIZE;
    int blockNum=curInode->dataBlocks[BlockId];                                     //Get the index of the datablock pointed to
    data_block_t* daBlock=(data_block_t*)(data_Block_Start+BLOCK_SIZE/4*blockNum);  //Get the datablock at index
    int i;
    for(i=0;i<length;i++,DataId++){
        if(DataId==BLOCK_SIZE){                               
            DataId=0;
        }
        memcpy(buf,&(daBlock->data[DataId]),1);
        buf++;
        if(DataId==BLOCK_SIZE-1){                                                       //If end of current datablock, get the next one
            BlockId++;
            blockNum=curInode->dataBlocks[BlockId];
            daBlock=(data_block_t*)(data_Block_Start+BLOCK_SIZE/4*blockNum);
        }
    }
    return length;
}





//--------------------------------------Files--------------------------------------------------

/***
 * file_open
 * adds a file to the PCB and allows future operations
 * INPUT: filename: path of file to open
 * OUTPUT: none
 * RETURN:0 for success, -1 for failure
 * SIDE EFFECTS: calls read_dentry_by_name, modifies PCB
 */
int32_t file_open(const uint8_t* filename){
    dentry_t de;
    PCB_t* PCB=terminal[cur_term].cur_PCB;
    //use read_dentry_by_name and verify file exists, get inode and type
    if (0 != read_dentry_by_name(filename, &de) )
        return -1;

    //find open fd in PCB, start at 2 since 0 and 1 are stdin and stdout
    int fd = 2;
    for (fd = 2; fd < PCB_SIZE; ++fd) {
        if( (PCB->File_Desc_Array[fd].flags & 0x1) == 0x1) {
            if (fd == PCB_SIZE - 1)
                return -1;  //couldnt find space
        }
        else
            break;
    }
    //fill in PCB data from dentry
    PCB->File_Desc_Array[fd].file_position = 0;
    PCB->File_Desc_Array[fd].flags = 0x1;
    PCB->File_Desc_Array[fd].inode = de.inode;
    //check file type

  
    return fd;           //fd??
}


/***
 * file_close
 * removes file from PCB
 * INPUT: fd: file descriptor for file
 * OUTPUT: none
 * RETURN: 0 for success, -1 for failure
 * SIDE EFFECTS: modifies PCB
 */
int32_t file_close(int32_t fd){
    //remove fd from PCB and set to available
    PCB_t* PCB=terminal[cur_term].cur_PCB;
    PCB->File_Desc_Array[fd].flags = PCB->File_Desc_Array[fd].flags & 0xFFFFFFFE; //set present flag bit to empty
    return 0;
}


/***
 * file_read
 * reads a certain number of bytes from a given file
 * INPUT:   fd: file descriptor for file
 *          nbytes: number of bytes to read from file
 * OUTPUT:  buf: buffer large enough to hold number of bytes read
 * RETURN:  number of bytes read for success, -1 for failure
 * SIDE EFFECTS: modifies PCB, calls read_data
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    //grab inode from PCB[fd]
    if (buf == NULL)
        return -1;
    if (0 == is_fd_active(fd))  //Check if file at fd is active
        return -1;
    PCB_t* PCB=terminal[cur_term].cur_PCB;
    int32_t inode = PCB->File_Desc_Array[fd].inode;
    //grab file position from PCB[fd]
    int32_t offset = PCB->File_Desc_Array[fd].file_position;
    //verify flags?
    
    //use read data
    int32_t ec = read_data(inode, offset, buf, nbytes);
    if ( ec == -1)
        return -1;
    {
        PCB->File_Desc_Array[fd].file_position += ec; //Increment file position by number of bytes read
        return ec;
    }   
        
}


/***
 * file_read
 * unimplemented
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    //later checkpoint
    return -1;
}



//------------------------------------Directories----------------------------------------------


/***
 * dir_open
 * adds a directory to the PCB and allows future operations
 * INPUT: filename: path of directory to open
 * OUTPUT: none
 * RETURN:0 for success, -1 for failure
 * SIDE EFFECTS: calls read_dentry_by_name, modifies PCB
 */
int32_t dir_open(const uint8_t* filename){

    dentry_t de;
    //use read_dentry_by_name and verify file exists, get inode and type
    if (0 != read_dentry_by_name(filename, &de))
        return -1;
    
    return 0;
}


/***
 * dir_close
 * removes directory from PCB
 * INPUT: fd: file descriptor for directory
 * OUTPUT: none
 * RETURN: 0 for success, -1 for failure
 * SIDE EFFECTS: modifies PCB
 */
int32_t dir_close(int32_t fd) {
    PCB_t* PCB=terminal[cur_term].cur_PCB;
    PCB->File_Desc_Array[fd].flags = PCB->File_Desc_Array[fd].flags & 0xFFFFFFFE; //set present flag bit to empty
    return 0;
}


/***
 * dir_read
 * reads the next entry in the directory and copies the first <length> bytes of its name
 * INPUT:   fd: file descriptor for file
 *          length: number of bytes to read from the entry name
 * OUTPUT:  buf: buffer large enough to hold number of bytes read
 * RETURN:  number of bytes read for success, -1 for failure, -2 for EOF
 * SIDE EFFECTS: modifies PCB, calls read_data
 */
int32_t dir_read(int32_t fd, void* buf, uint32_t length){
    PCB_t* PCB=terminal[cur_term].cur_PCB;
    uint32_t Pos=PCB->File_Desc_Array[fd].file_position;                 //Get the file position to read to
    if (buf == NULL)
        return 0; //SRAM
    if (0 == is_fd_active(fd))
        return 0; //SRAM
    if(Pos>=bootBlock->stats.numDirEntries){ 
        return 0;  //Reached end of directory entries //SRAM
    }
    dentry_t myDentry=bootBlock->dirEntries[Pos]; //Get current dentry  
    PCB->File_Desc_Array[fd].file_position++;                
    memcpy(buf,myDentry.name,length);               //Copy name of file to buffer
    return length;
}


/***
 * dir write
 * unimplemented
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}


/***
 * get_inode_start
 * returns requested value, its in the name, bruh
 */
uint32_t* get_inode_start(){
    return iNode_Start;
}
