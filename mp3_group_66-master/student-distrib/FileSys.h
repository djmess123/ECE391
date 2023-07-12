#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"
//#include "ece391support.h"
#include "lib.h"


#define BLOCK_SIZE 4096
#define NUM_INODES 64
#define PCB_SIZE 8
#define NAME_LENGTH 32
#define DENTRY_LENGTH 64
#define MAX_FILE_NUM 8



/***
 * dentry_t
 * struct for directory entry
 */
typedef struct __attribute__((aligned(DENTRY_LENGTH))) dentry {
    int8_t name[NAME_LENGTH];             //32 Byte FileName
    uint32_t fileType;                      //4 Byte FileType
    uint32_t inode;                 //4 Byte Inode Index
} dentry_t;  


/***
 * statistics_t
 * struct for boot block information
 */
typedef struct __attribute__((aligned(DENTRY_LENGTH))) statistics {
    uint32_t numDirEntries;         //4 Byte Number of directory entries  
    uint32_t numInodes;             //4 Byte Number of Inodes
    uint32_t numDataBlocks;         //4 Byte Number of DataBlocks
} statistics_t;


/***
 * boot_block_t
 * struct to boot block
 */
typedef struct __attribute__((aligned(BLOCK_SIZE)))  boot_block {
    statistics_t stats;             //First 64 bytes are stats
    dentry_t dirEntries[DENTRY_LENGTH-1];        //Room for up to 63 dirEntries
} boot_block_t;


/***
 * inode_t
 * struct for inode
 */
typedef struct __attribute__((aligned(BLOCK_SIZE))) inode {
    uint32_t blockLength;           //Length of dataBlock for file in Bytes
    uint32_t dataBlocks[DENTRY_LENGTH];        //Array of Datablocks for files
} inode_t;


/***
 * data_block_t
 * struct for data block
 */
typedef struct __attribute__((aligned(BLOCK_SIZE))) data_block {
    uint8_t data[BLOCK_SIZE];         //4096 Bytes in a data block
} data_block_t;

/***
 * optable structure 
 */
typedef struct OP_TABLE{
    uint32_t (*read)(int32_t,void*,int32_t);
    uint32_t (*write)(int32_t, void*,int32_t);
    uint32_t (*open)(uint8_t*);
    uint32_t (*close)(int32_t);
} OP_TABLE_t;

/***
 * data_block_t
 * temprorary struct for PCB
 */
typedef struct File_Descriptor_Array{
    OP_TABLE_t * optable_array; //TODO: put in Optable structure here
    uint32_t inode;             //Inode number for read files
    uint32_t file_position;     //place in file to next read from
    uint32_t flags;             //bit 1 means in use
} File_Descriptor_Array_t;



/* Process control block */
/***
 * Holds data for all currently running processes, 
 * each have unique ID, and hold data regarding to vidmap,
 * saved image for halt, argument text, and file descriptors
 */
typedef struct PCB{
    uint32_t Pid;
    int32_t Parent_ID;
    uint8_t pcb_arg_buf[128];
    uint32_t pcb_arg_length;
    File_Descriptor_Array_t File_Desc_Array[8];
    uint32_t Saved_Esp;
    uint32_t Saved_Ebp;
    uint32_t active; 
    uint32_t vidmap;
} PCB_t;

//init
int fileSystem_init(uint32_t * fs_start);

//helper functions
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

//file functions
extern int32_t file_open(const uint8_t* filename);
extern int32_t file_close(int32_t fd);
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

//directory functions
extern int32_t dir_open(const uint8_t* filename);
extern int32_t dir_close(int32_t fd);
extern int32_t dir_read(int32_t fd, void* buf, uint32_t length);
extern int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);


uint32_t* get_inode_start();


#endif /*_FILESYS_H*/
