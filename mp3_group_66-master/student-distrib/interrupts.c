//TODO: add descriptor for file
#include "lib.h"
#include "interrupts.h"
#include "x86_desc.h"
#include "link.h"
#include "syscalls.h"


#define KEYBOARD_ENTRY 0x21
#define RTC_ENTRY 0x28
#define SYS_ENTRY 0x80
#define PIT_ENTRY 0x20
#define HANG_ON_EXCEPTION 1

/***
*
*INPUTs: None
*OUPUTs: None
*SIDE EFFECTS:initialize interrupt descriptor table with vectors
*/
void idt_init(){
    int i;
    // initialize exceptions
    for(i = 0; i < 20 ; i++){
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl = 0;
        idt[i].present = 1;
    }
    //selector for keyboard
    idt[KEYBOARD_ENTRY].seg_selector = KERNEL_CS;
    idt[KEYBOARD_ENTRY].reserved4 = 0;
    idt[KEYBOARD_ENTRY].reserved3 = 0;
    idt[KEYBOARD_ENTRY].reserved2 = 1;
    idt[KEYBOARD_ENTRY].reserved1 = 1;
    idt[KEYBOARD_ENTRY].size = 1;
    idt[KEYBOARD_ENTRY].reserved0 = 0;
    idt[KEYBOARD_ENTRY].dpl = 0;
    idt[KEYBOARD_ENTRY].present = 1;
    //selector for RTC 
    idt[RTC_ENTRY].seg_selector = KERNEL_CS;
    idt[RTC_ENTRY].reserved4 = 0;
    idt[RTC_ENTRY].reserved3 = 0;
    idt[RTC_ENTRY].reserved2 = 1;
    idt[RTC_ENTRY].reserved1 = 1;
    idt[RTC_ENTRY].size = 1;
    idt[RTC_ENTRY].reserved0 = 0;
    idt[RTC_ENTRY].dpl = 0;
    idt[RTC_ENTRY].present = 1;
    //Selector for System Calls
    idt[SYS_ENTRY].seg_selector = KERNEL_CS;
    idt[SYS_ENTRY].reserved4 = 0;
    idt[SYS_ENTRY].reserved3 = 1;
    idt[SYS_ENTRY].reserved2 = 1;
    idt[SYS_ENTRY].reserved1 = 1;
    idt[SYS_ENTRY].size = 1;
    idt[SYS_ENTRY].reserved0 = 0;
    idt[SYS_ENTRY].dpl = 3;
    idt[SYS_ENTRY].present = 1;

    //Selector for System Calls
    idt[PIT_ENTRY].seg_selector = KERNEL_CS;
    idt[PIT_ENTRY].reserved4 = 0;
    idt[PIT_ENTRY].reserved3 = 0;
    idt[PIT_ENTRY].reserved2 = 1;
    idt[PIT_ENTRY].reserved1 = 1;
    idt[PIT_ENTRY].size = 1;
    idt[PIT_ENTRY].reserved0 = 0;
    idt[PIT_ENTRY].dpl = 0;
    idt[PIT_ENTRY].present = 1;

    //system linkage functions
    SET_IDT_ENTRY(idt[0], divide_by_zero_linkage);
    SET_IDT_ENTRY(idt[1], single_step_linkage);
    SET_IDT_ENTRY(idt[2], nmi_interrupt_linkage);
    SET_IDT_ENTRY(idt[3], breakpoint_linkage);
    SET_IDT_ENTRY(idt[4], overflow_linkage);
    SET_IDT_ENTRY(idt[5], bound_exceed_linkage);
    SET_IDT_ENTRY(idt[6], invalid_opcode_linkage);
    SET_IDT_ENTRY(idt[7], device_not_available_linkage);
    SET_IDT_ENTRY(idt[8], double_fault_linkage);
    SET_IDT_ENTRY(idt[9], coprocessor_overrun_linkage);
    SET_IDT_ENTRY(idt[10], invalid_tts_linkage);
    SET_IDT_ENTRY(idt[11], segment_not_present_linkage);
    SET_IDT_ENTRY(idt[12], stack_segment_fault_linkage);
    SET_IDT_ENTRY(idt[13], general_protection_linkage);
    SET_IDT_ENTRY(idt[14], page_fault_linkage);
    SET_IDT_ENTRY(idt[15], reserved_linkage);
    SET_IDT_ENTRY(idt[16], floating_point_error_linkage);
    SET_IDT_ENTRY(idt[17], alignment_check_error_linkage);
    SET_IDT_ENTRY(idt[18], machine_check_linkage);
    SET_IDT_ENTRY(idt[19], floating_point_exception_linkage);
    // TODO: figure out what to fill these last two with/if they need to be here
    SET_IDT_ENTRY(idt[20], reserved_linkage);
    SET_IDT_ENTRY(idt[21], reserved_linkage);
    
    // 0 - 32 reserved for cpu exceptions

    //Set keyboard idt entry. port 33
    SET_IDT_ENTRY(idt[PIT_ENTRY], pit_handler_linkage);
    SET_IDT_ENTRY(idt[KEYBOARD_ENTRY], kb_handler_linkage);
    SET_IDT_ENTRY(idt[RTC_ENTRY], rtc_handler_linkage);
    SET_IDT_ENTRY(idt[SYS_ENTRY], syscall_linkage);
    
}

//--------------------------------
//BLOCK FUNCTION HEADER FOR ALL FUNCTIONS BELOW (print statements subject to change)
//--------------------------------


/***
* Exception Handler Functions
*INPUTs: None
*OUPUTs: None
*SIDE EFFECTS: Prints statements to screen to show interrupt happening.
*/

void divide_by_zero_handler(){
    printf("Divide By Zero Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1); //TODO: insert halt here
}

void single_step_handler(){
    printf("Single Step Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}

void nmi_interrupt_handler(){
    printf("NMI Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void breakpoint_handler(){
    printf("Breakpoint Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void overflow_handler(){
    printf("Overflow Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void bound_exceed_handler(){
    printf("Bound Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void invalid_opcode_handler(){
    printf("Invalid Opcode Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void device_not_available_handler(){
    printf("Device Not Available Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void double_fault_handler(){
    printf("Double Fault Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void coprocessor_overrun_handler(){
    printf("Coprocessor Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void invalid_tts_handler(){
    printf("TTS Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void segment_not_present_handler(){
    printf("Segment Not Present Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void stack_segment_fault_handler(){
    printf("Stack Segment Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void general_protection_handler(){
    printf("General Protection Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void page_fault_handler(){
    printf("Page Fault Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void floating_point_error_handler(){
    printf("Floating Point Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}
void alignment_check_handler(){
    printf("Alignment Check Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}

void machine_check_handler(){
    printf("Machine Check Handler\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}

void floating_point_exception_handler(){
    printf("Floating Point Error\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}

void reserved_handler(){
    printf("Reserved Handler\n");
    if (HANG_ON_EXCEPTION) while(1) {}
    halt_flag = 1;
    system_halt(-1);
}

