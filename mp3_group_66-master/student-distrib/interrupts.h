//TODO: write file header
#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H



//--------------------------------
//BLOCK FUNCTION HEADER FOR ALL FUNCTIONS BELOW (print statements subject to change)
//--------------------------------


/***
* Exception Handler Functions
*INPUTs: None
*OUPUTs: None
*SIDE EFFECTS: Prints statements to screen to show interrupt happening.
*/

extern void idt_init();

void divide_by_zero_handler();

void single_step_handler();

void nmi_interrupt_handler();

void breakpoint_handler();

void overflow_handler();

void bound_exceed_handler();

void invalid_opcode_handler();

void device_not_available_handler();

void double_fault_handler();

void coprocessor_overrun_handler();

void invalid_tts_handler();

void segment_not_present_handler();

void stack_segment_fault_handler();

void general_protection_handler();

void page_fault_handler();

void floating_point_error_handler();

void alignment_check_error_handler();

void floating_point_error_handler();

void machine_check_handler();

void floating_point_exception_handler();

void reserved_handler();



#endif  /* _INTERUPTS_H */ 

