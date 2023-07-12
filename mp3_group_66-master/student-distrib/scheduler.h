#include "lib.h"
#include "FileSys.h"
#include "rtc.h"
#include "terminal.h"
#include "i8259.h"
#include "Paging.h"
#include "x86_desc.h"
#include "syscalls.h"
#include "keyboard.h"

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#define DBGTXT_SCH 0

//scheduler function called by pit
void scheduler();

//helper function that saves and loads all required values
void next_context();

//called when PIT timer expires for current process
int32_t pit_callback();





#endif  /*_SCHEDULER_H */ 
