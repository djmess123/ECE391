#include "syscalls.h"
#include "FileSys.h"
#include "i8259.h"
#include "terminal.h"
#include "keyboard.h"

/*
    #defineine PR(f_,...) (DBGTXT?printf((f_),##__VA_ARGS__):void())
*/

//debug printing macro
#define PR(fmt, ...) \
            do { if (DBGTXT) printf(fmt, ##__VA_ARGS__); } while (0)



OP_TABLE_t rtc_optable;
OP_TABLE_t dir_optable;
OP_TABLE_t filesys_optable;
OP_TABLE_t stdin_optable;
OP_TABLE_t stdout_optable;



//PID Managment System --------------------------------------
int32_t process_id=0; //when process id is /* might want to rename to process number, maybe use global PCB */
int32_t _pid_array[MAX_PROCESSES];
int32_t halt_ret;


//memset(_pid_array,0,sizeof(_pid_array));

//Note: PID 1 is for the shell
int32_t halt_flag = 0;


/***
 * gets an open PID, is 1 indexed
 * INPUT: None
 * OUTPUT: returns -1 if no more processes available, returns open pid number if one is available
 * EFFECTS: set the pid to be occupied
 */

int32_t get_open_pid(){
    int i;
    for(i = 0; i < MAX_PROCESSES; i++){
        if(_pid_array[i]==0){
            _pid_array[i] = 1;
            return i+1;
        }
    }
    return -1; //If no PID's are availabe
}

int32_t close_pid(int32_t pid){
    if((pid >= 1)&&(pid <= MAX_PROCESSES)){
        _pid_array[pid-1] = 0;
        return 1;
    }
    return -1; 
}


//System functions----------------------
/* system_init */
/***
 * initilizes files and functions 
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: edits optables
 */
int32_t system_init(){
    int i;
    // initialize pid array to zero
    for(i = 0; i < MAX_PROCESSES; i++){
        _pid_array[i]=0;
    }
    //Initialize rtc
    rtc_optable.read = (void*)rtc_read;
    rtc_optable.write = (void*)rtc_write;
    rtc_optable.open = (void*)rtc_open;
    rtc_optable.close = (void*)rtc_close;
    //Initialize dir
    dir_optable.read = (void*)dir_read;
    dir_optable.write = (void*)dir_write;
    dir_optable.open = (void*)dir_open;
    dir_optable.close = (void*)dir_close;
    //Initialize file
    filesys_optable.read = (void*)file_read;
    filesys_optable.write = (void*)file_write;
    filesys_optable.open = (void*)file_open;
    filesys_optable.close = (void*)file_close;
    //Initialize in
    stdin_optable.read = (void*)read_terminal;
    stdin_optable.write = (void*)NULL;
    stdin_optable.open = (void*)NULL;
    stdin_optable.close = (void*)NULL;
    //Initialize out
    stdout_optable.read = (void*)NULL;
    stdout_optable.write = (void*)write_terminal;
    stdout_optable.open = (void*)NULL;
    stdout_optable.close = (void*)NULL;
    return 1;
}

/**
 * system call for execute 
 * INPUT: pointer to command to execute
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: 
 */

int32_t system_execute(const uint8_t* command) {
    cli();
    PR("-------------------------------EXECUTE------------------------------\n");
    //PR("Cur_pcb->PID %d PCB->Parent_PID %d",cur_PCB->Pid,cur_PCB->Parent_ID);
    uint8_t buf[32];
    int i=0;
    int fd=2;
    int32_t new_pid;
    int32_t argument_length = 0;
    int8_t argument_buf[128];

    //check if the maximum amount of processes has been reached
    new_pid = get_open_pid();
    if (new_pid == -1) {
        printf("Maximum Processes Reached\n");
        return -1; // double check return value if we reach cap
    }

    //Parse Cmd
    while(command[i]!=' '&& i<strlen((int8_t*)command)){
        buf[i]=command[i];
        i++;
    }
    buf[i] = 0x00; //null terminate

    //Parse Arg
    int32_t numspace = 0;
    while(command[i+argument_length] != '\n' && command[i+argument_length] != '\0'){
        if(command[i+argument_length] != ' '){
            argument_buf[argument_length-numspace]=command[i+argument_length];
        }
        else{
            numspace++;
        }
        argument_length++;
        if(argument_length>128) return -1;
    }
      argument_buf[argument_length-numspace]=0x00; //null terminate
    //argument_length--;
    PR("Argument Parse: %s  @ length: %d\n", argument_buf, argument_length-numspace);

    //File Checks
    dentry_t temp_dentry;
    
  
    //Check for Existence
    if(read_dentry_by_name(buf,&temp_dentry)==-1){
        close_pid(new_pid);
        PR("dentry read by name failed\n");
        return -1;   //If file is not found, return -1
    }

    uint32_t* iNode_Start=get_inode_start();
    inode_t* curInode=(inode_t*)(iNode_Start+BLOCK_SIZE/4*(temp_dentry.inode)); 
    uint32_t length=curInode->blockLength;

    //Check for validity
    uint8_t fileBuf[32];
    if(read_data(temp_dentry.inode,0,fileBuf,32)<=0){
        PR("inode validity check failed\n");
        close_pid(new_pid);
        return -1; //File is not valid, return -1.
    }

    //Check for executable
    if(fileBuf[0]!=0x7F||fileBuf[1]!=0x45||fileBuf[2]!=0x4c||fileBuf[3]!=0x46){
        PR("no executable found\n");
        close_pid(new_pid);
        return -1; //If file is not an executable, return -1;
    }

    //good to go, set up PCB and keep going!
    PCB_t * new_PCB = (PCB_t*)PCB_ADDR(new_pid);
    

    new_PCB->active = 1;
    new_PCB->Pid = new_pid;
    if (new_PCB == NULL) {
        printf("null PCB pointer\n");
        return ABNORMAL_EXIT;
    }
    else if (terminal[cur_term].cur_PCB == NULL)
    {
        new_PCB->Parent_ID = 0; //makes the new_PCB the "base shell"
    }
    else {
        new_PCB->Parent_ID = terminal[cur_term].cur_PCB->Pid; //locate what pcb we are in and find its parent **CP 5**
    }

    PR("New_PCB on PID: %d @add: %x with parent_pid: %d\n", new_pid, new_PCB, new_PCB->Parent_ID);

    //setup std in in 1st file directory
    new_PCB->File_Desc_Array[0].optable_array=&stdin_optable;
    new_PCB->File_Desc_Array[0].flags=1;                  //
    new_PCB->File_Desc_Array[0].file_position=0;
    new_PCB->File_Desc_Array[0].inode=0;

    //setup std out in 2nd file directory
    new_PCB->File_Desc_Array[1].optable_array=&stdout_optable;
    new_PCB->File_Desc_Array[1].flags=1; 
    new_PCB->File_Desc_Array[1].file_position=0;
    new_PCB->File_Desc_Array[1].inode=0;

    //setup remaining file directories
    for(fd=2;fd<8;fd++){
        new_PCB->File_Desc_Array[fd].optable_array=NULL;
        new_PCB->File_Desc_Array[fd].flags=0; 
        new_PCB->File_Desc_Array[fd].file_position=0;
        new_PCB->File_Desc_Array[fd].inode=0;
    }
    //fill arguments and size for current pcb
    int k;
    new_PCB->pcb_arg_length = argument_length;
    for(k=0; k<argument_length; k++){
        new_PCB->pcb_arg_buf[k] = argument_buf[k];
    }


    //fill arguments and size for current pcb
    new_PCB->pcb_arg_length = argument_length;
    for(k=0; k<32; k++){
        new_PCB->pcb_arg_buf[k] = argument_buf[k];
    }

    //setup paging and other stuff
    uint32_t paging_add = PAGE_ADDR(new_PCB->Pid);     //DJM bookmark this is where we  set up paging for the PCB stack...? should it be -1Byte to not overflow next 4KB page?
    uint8_t* Img_add=(uint8_t*)(VIRTUAL_LOC+IMG_OFFSET);

    PR("New_Paging setup address: %x  IMG: %x\n", paging_add, Img_add);
    setup_paging(paging_add);                       //set page table to new PCB page??
    flush_tlb();
    read_data(temp_dentry.inode,0,Img_add,length);  //copy program data into program_IMG

    //Get Instruction Pointer (aka where to start user program)
    uint32_t eip;
    uint8_t Prog_eip[4];
    read_data(temp_dentry.inode,24,Prog_eip,4);
    eip=(uint32_t)(&Prog_eip);
    
   
    //save stack stuff for current PCB, unless its the base frame cause that has nothing to return to
    register uint32_t saved_ebp asm("%ebp");
    register uint32_t saved_esp asm("%esp");
    if (new_PCB->Parent_ID > 0)
    {
        terminal[cur_term].cur_PCB->Saved_Esp = saved_esp;    //soon to be parent sets its return value so we can return to it later
        terminal[cur_term].cur_PCB->Saved_Ebp = saved_ebp;
    }

    //save kernal stack pointer
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KSTACK_ADDR(new_PCB->Pid);         //DJM bookmark   why -4? wouldnt this overwrite the PCB_t? -284?
    //tss.esp0 = saved_esp;


    terminal[cur_term].cur_PCB = new_PCB;                //set new process as the current/active
    

    PR("SAVE return stack info TO PARENT: esp: %x   ebp: %x\n", saved_esp, saved_ebp);
    PR("New process k-stack pointer (tss.esp0): %x\n", tss.esp0);
    PR("------------------------------jumping to USER ---------------------------- \n");
    /*  @LUCA write down the ebx value */ 
    jump_usermode(eip);
    /*  @LUCA check ebx value, in theory they should be the same */ 
    sti();
    //return from halt to execute's return
    asm volatile ("execute_return:");
    asm volatile ("leave");                                     //bookmark?
    asm volatile ("ret");
    PR("RETURN FROM EXECUTE halt_ret = %d\n", halt_ret);
    return halt_ret;
}

/**
 * system call for halt 
 * INPUT: status
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: 
 */
int32_t system_halt(uint8_t status){
    
    cli();
    PCB_t * PCB_parent;
    int i;
    
    PR("\n---------------------------------HALT---------------------------------\n");


    /* close all of the active processes and set currently active processes to non active */
    
    //iterate through all the file desc array within each pcb block
    for(i = 2; i < 8; i++){
        if(terminal[cur_term].cur_PCB->File_Desc_Array[i].flags == 1){
            terminal[cur_term].cur_PCB->File_Desc_Array[i].flags = 0;
            if(terminal[cur_term].cur_PCB->File_Desc_Array[i].optable_array!=NULL){
                terminal[cur_term].cur_PCB->File_Desc_Array[i].optable_array->close(i);         //CLOSE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            }
        }
    }
    //set active flag to 0 and close PID
    terminal[cur_term].cur_PCB->active = 0;
    close_pid(terminal[cur_term].cur_PCB->Pid);
    PR("PCB (pid=%d) closed!\n", terminal[cur_term].cur_PCB->Pid);

    /* check if main shell, restart root shell */     //bookmark, check that logic works on second time around
    if(terminal[cur_term].cur_PCB->Parent_ID == 0){
        PR("Base shell requested HALT. Restarting shell...\n");
        terminal[cur_term].cur_PCB = NULL;
        return system_execute((uint8_t*)"shell");
    } //TODO: should there be an else here?

    

    PR("Restoring parent process (pid=%d)\n",terminal[cur_term].cur_PCB->Parent_ID);
    //get parent process
    PCB_parent = (PCB_t*)PCB_ADDR(terminal[cur_term].cur_PCB->Parent_ID); 

    //set tss for parent
    tss.ss0 = KERNEL_DS;
    //set the parent process esp and ebp from the tss?
    tss.esp0 = KSTACK_ADDR(PCB_parent->Pid);  
    PR("Parent process k-stack pointer (tss.esp0): %x\n", tss.esp0);  
    
    
    //map parents paging     */
    uint32_t parent_add=PAGE_ADDR(PCB_parent->Pid);
    PR("Parent Paging setup address: %x\n", parent_add);
    setup_paging(parent_add);
    flush_tlb();
    

    /* halt return (asm) */
    //status check, if there is exception raised, return 256 error code.  if no exception raised, return the status.  
    if(halt_flag == 1){
        halt_ret = 256;
        halt_flag = 0;
    }
    else{
        halt_ret = status;
    }


    //load parent as the new active PCB
    PR("LOAD from parent pid(%d): esp: %x  ebp: %x\n",PCB_parent->Pid, PCB_parent->Saved_Esp, PCB_parent->Saved_Ebp);
    PCB_parent->active = 1;
    terminal[cur_term].cur_PCB = PCB_parent;
    PR("Parent now ACTIVE and set as cur_PCB!\n");
    
    //load saved values into registers
    PR("%d",halt_ret);
    __asm__ __volatile__(
            "movl %0, %%esp \n\t"
            :
            :"a"(terminal[cur_term].cur_PCB->Saved_Esp)
            :"%esp"
    );
    __asm__ __volatile__(
            "movl %0, %%ebp \n\t"
            :
            :"a"(terminal[cur_term].cur_PCB->Saved_Ebp)
            :"%ebp"
    );

    __asm__ __volatile__(
            "movl %0, %%eax \n\t"
            :
            :"c"(halt_ret)
            :"%eax"
    );
    sti();
    
    //jump to execute and return
    __asm__ __volatile__(
            "jmp execute_return \n\t"
    );


    printf("YER DONE (should never get here 356\n");

    return halt_ret; 
}

/**
 * system call for read 
 * INPUT: file descriptor, buffer, number of bytes to use
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: calls function from an op table based on pointer 
 */
int32_t system_read(int32_t fd, const void* buf, int32_t nbytes){

    //verify number of bytes
    if(nbytes < 0){
        return -1;
    }

    //find the current pcb and check if it's open
    PCB_t * new_PCB = terminal[cur_term].cur_PCB;
    if( (new_PCB->File_Desc_Array[fd].flags & 0x1) != 0x1) {
        return -1;
    }
    else if (new_PCB->File_Desc_Array[fd].optable_array->read==NULL)
    {
        return -1;
    }
    //check if the buf is not null
    if(buf == NULL){
        return -1;
    }

    //fd in range check
    if(fd < 0 || fd > 7){
        return -1;
    }

    //call the correct function
    

    //set up return for read to return the FD
    return new_PCB->File_Desc_Array[fd].optable_array->read(fd, (uint8_t*)buf, nbytes);
}

/**
 * system call for write 
 * INPUT: file descriptor, buffer, number of bytes to use
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: calls function from an op table based on pointer
 */
int32_t system_write(int32_t fd, const void* buf, int32_t nbytes){
    //verify number of bytes
    if(nbytes < 0){
        return -1;
    }
      
    //check if the buf is not null
    if(buf == NULL){
        return -1;
    }

    //fd in range check
    if(fd < 0 || fd > 7){
        return -1;
    }

    //find the current pcb and check if it's open
    PCB_t * new_PCB = terminal[cur_term].cur_PCB;
    if( (new_PCB->File_Desc_Array[fd].flags & 0x1) != 0x1) {
        return -1;
    }
    else if (new_PCB->File_Desc_Array[fd].optable_array->write==NULL)
    {
        return -1;
    }

    return new_PCB->File_Desc_Array[fd].optable_array->write(fd, (uint8_t*)buf, nbytes);
}

/**
 * system call for open 
 * INPUT: 
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: calls function from an op table based on pointer
 */
int32_t system_open(const uint8_t* filename){
    dentry_t de;
    PCB_t * My_PCB = terminal[cur_term].cur_PCB;
    uint8_t buf[32];
    int i=0;
    while(filename[i]!='\0'&& i<strlen((int8_t*)filename)){
        buf[i]=filename[i];
        i++;
    }
    buf[i]=0x00;
    //use read_dentry_by_name and verify file exists, get inode and type
    if (0 != read_dentry_by_name(buf, &de) )
        return -1;
    //Find the pcb
    
    //find open fd in PCB, start at 2 since 0 and 1 are stdin and stdout
    int fd = 2;
    for (fd = 2; fd < PCB_SIZE; ++fd) {
        if( (My_PCB->File_Desc_Array[fd].flags & 0x1) == 0x1) {
            if (fd == PCB_SIZE - 1)
                return -1;  //couldnt find space
        }
        else
            break;
    }
    //fill in PCB data from dentry
    My_PCB->File_Desc_Array[fd].file_position = 0;
    My_PCB->File_Desc_Array[fd].flags = 0x1;
    My_PCB->File_Desc_Array[fd].inode = de.inode;

    //check file type
    //fill in the op table pointer based off of the file type
    switch(de.fileType){
        case 0:
            My_PCB->File_Desc_Array[fd].optable_array = &rtc_optable;
            My_PCB->File_Desc_Array[fd].inode = 0;
            break;
        case 1:
            My_PCB->File_Desc_Array[fd].optable_array = &dir_optable;
            My_PCB->File_Desc_Array[fd].inode = 0;
            break;
        case 2:
            My_PCB->File_Desc_Array[fd].optable_array = &filesys_optable;
            My_PCB->File_Desc_Array[fd].inode = de.inode;
            break;
    } 
    if (My_PCB->File_Desc_Array[fd].optable_array->open==NULL)
    {
        return -1;
    }


    //call the correct function
    My_PCB->File_Desc_Array[fd].optable_array->open((uint8_t*)buf);

    //set up return for open function to return the FD
    return fd;
}

/**
 * system call for close 
 * INPUT: 
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: calls function from an op table based on pointer
 */
int32_t system_close(int32_t fd){
    //Check invalid inpute
    PCB_t* My_PCB=terminal[cur_term].cur_PCB;
    if((fd < 0) || (fd > 7 )){
        return -1;
    }
    else if (My_PCB->File_Desc_Array[fd].optable_array->close==NULL||My_PCB->File_Desc_Array[fd].flags==0)
    {
        return -1;
    }
    PCB_t * PCB_Add = terminal[cur_term].cur_PCB;
    if(PCB_Add->File_Desc_Array[fd].optable_array!=NULL)
    {
        PCB_Add->File_Desc_Array[fd].optable_array->close(fd);
    }
    PCB_Add->File_Desc_Array[fd].flags = 0;
    return fd;
}

//SYSTEM CALLS FOR CP4----------------------------------------------

/**
 * system call for getargs 
 * INPUT: 
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: 
 */
int32_t system_getargs(uint8_t* buf, int32_t nbytes){
    //get the current process
    PCB_t* PCB_temp = terminal[cur_term].cur_PCB;
    int32_t arg_length = terminal[cur_term].cur_PCB->pcb_arg_length;
    if(PCB_temp == NULL){
        return -1;
    }
      
    //check if the buf is not null
    if(buf == NULL){
        return -1;
    }

    //verify number of bytes
    if(arg_length > nbytes){
        return -1;
    }
    
    //fill the buffer with the arguments found from execute
    int i;
    for (i = 0; i < arg_length; i++){
        buf[i] = terminal[cur_term].cur_PCB->pcb_arg_buf[i];
    }
    
    return 0;
}

/**
 * system call for vidmap 
 * INPUT: 
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: 
 */
int32_t system_vidmap(uint8_t** screen_start){
    if(NULL==screen_start) return -1; //NULL check
    //int add=(uint32_t)screen_start;
    uint32_t low_end = VIRTUAL_LOC;
    uint32_t high_end= VIRTUAL_LOC+FOURMB;
    if((uint32_t)screen_start<low_end||(uint32_t)screen_start>high_end){
        return -1;
    }
    terminal[cur_term].cur_PCB->vidmap=1; //Set vidmap active
    pageDir[33].tabAddress =(uint32_t)pageTable|7;  //Put the pagetable for user video
    pageTable[0].page=((uint32_t)VIDEO|7); 
    flush_tlb();
    *screen_start=(uint8_t*)high_end;
    return high_end;
}

/**
 * system call for set_handler 
 * INPUT: 
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: 
 */
int32_t system_set_handler (int32_t signum, void* handler_address){

    return 0;
}

/**
 * system call for sigreturn 
 * INPUT: 
 * OUTPUT: 0 on success -1 on failure
 * SIDE EFFECTS: 
 */
int32_t system_sigreturn (void){

    return 0;
}





