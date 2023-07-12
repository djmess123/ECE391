#if !defined(LINK_H)
#define LINK_H


// rtc handler linkage function, called in link.s to link handler functions
extern void rtc_handler_linkage();

//Keyboard handler linkage function, called in link.s to link handler functions
extern void kb_handler_linkage();

//PIT handler linkage function, called in link.s to link handler functions
extern void pit_handler_linkage();

// syscall linkage function 
extern void syscall_linkage();

/*interrupt linkage*/
extern void divide_by_zero_linkage();

/*interrupt linkage*/
extern void single_step_linkage();

/*interrupt linkage*/
extern void nmi_interrupt_linkage();

/*interrupt linkage*/
extern void breakpoint_linkage();

/*interrupt linkage*/
extern void overflow_linkage();

/*interrupt linkage*/
extern void bound_exceed_linkage();

/*interrupt linkage*/
extern void invalid_opcode_linkage();

/*interrupt linkage*/
extern void device_not_available_linkage();

/*interrupt linkage*/
extern void double_fault_linkage();

/*interrupt linkage*/
extern void coprocessor_overrun_linkage();

/*interrupt linkage*/
extern void invalid_tts_linkage();

/*interrupt linkage*/
extern void segment_not_present_linkage();

/*interrupt linkage*/
extern void stack_segment_fault_linkage();

/*interrupt linkage*/
extern void general_protection_linkage();

/*interrupt linkage*/
extern void page_fault_linkage();

/*interrupt linkage*/
extern void floating_point_error_linkage();

/*interrupt linkage*/
extern void alignment_check_error_linkage();

/*interrupt linkage*/
extern void machine_check_linkage();

/*interrupt linkage*/
extern void floating_point_exception_linkage();

/*interrupt linkage*/
extern void reserved_linkage();




#endif /* LINK_H */
