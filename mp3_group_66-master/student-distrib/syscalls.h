#include "lib.h"
#include "FileSys.h"
#include "rtc.h"
#include "terminal.h"
#include "Paging.h"
#include "x86_desc.h"

#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#define DBGTXT 0

#define KERNEL_END   0x00800000
#define PROG_SIZE    0x00400000
#define IMG_OFFSET   0x00048000
#define EIGHTKB      0x2000
#define MAX_PROCESSES 6 //TODO: find out what the max number of process
#define PID_NOT_AVAILABLE -1
#define PROGRAM_NOT_FOUND -1
#define ABNORMAL_EXIT 0






#define PCB_ADDR(PID) (KERNEL_END - EIGHTKB*(PID))
#define PAGE_ADDR(PID) (KERNEL_END + ((PID)) * PROG_SIZE)
#define KSTACK_ADDR(PID) (KERNEL_END - EIGHTKB*((PID)-1) - 4)



extern int halt_flag;

//PCB_t* cur_PCB;

/* initialize system variables */
int32_t system_init();

/* system call for execute */
int32_t system_execute(const uint8_t* command);

/* system call for halt */
int32_t system_halt(uint8_t status);

/* system call for read */
int32_t system_read(int32_t fd, const void* buf, int32_t nbytes);

/* system call for write */
int32_t system_write(int32_t fd, const void* buf, int32_t nbytes);

/* system call for open */
int32_t system_open(const uint8_t* filename);

/* system call for close */
int32_t system_close(int32_t fd);

//SYSTEM CALLS FOR CP4------------------------------

/* system call for getargs */
int32_t system_getargs (uint8_t* buf, int32_t nbytes);

/* system call for vimap */
int32_t system_vidmap (uint8_t** screen_start);

/* system call for set_handler */
int32_t system_set_handler (int32_t signum, void* handler_address);

/* system call for sigreturn */
int32_t system_sigreturn (void);

extern void jump_usermode(uint32_t EIP);

#endif  /*_SYSCALLS_H */ 
