.GLOBL syscall_linkage

#define INTR_LINK(name, func)   \
.GLOBL name              ;\
name:   PUSHAL          ;\
    PUSHFL          ;\
	call func       ;\
	POPFL           ;\
	POPAL           ;\
    IRET;


INTR_LINK(pit_handler_linkage, handle_pit_interrupt);
INTR_LINK(kb_handler_linkage, handle_keyboard_interrupt);
INTR_LINK(rtc_handler_linkage, handle_rtc_interrupt);


#define COMMON_INT_LINK(name,func)   \
.GLOBL name 		;\
name:	PUSHAL          ;\
	call func       ;\
	POPAL           ;\
    IRET;

COMMON_INT_LINK(divide_by_zero_linkage, divide_by_zero_handler);
COMMON_INT_LINK(single_step_linkage, single_step_handler);
COMMON_INT_LINK(nmi_interrupt_linkage, nmi_interrupt_handler);
COMMON_INT_LINK(breakpoint_linkage, breakpoint_handler);
COMMON_INT_LINK(overflow_linkage, overflow_handler);
COMMON_INT_LINK(bound_exceed_linkage, bound_exceed_handler);
COMMON_INT_LINK(invalid_opcode_linkage, invalid_opcode_handler);
COMMON_INT_LINK(device_not_available_linkage, device_not_available_handler);
COMMON_INT_LINK(double_fault_linkage, double_fault_handler);
COMMON_INT_LINK(coprocessor_overrun_linkage, coprocessor_overrun_handler);
COMMON_INT_LINK(invalid_tts_linkage, invalid_tts_handler);
COMMON_INT_LINK(segment_not_present_linkage, segment_not_present_handler);
COMMON_INT_LINK(stack_segment_fault_linkage, stack_segment_fault_handler);
COMMON_INT_LINK(general_protection_linkage, general_protection_handler);
COMMON_INT_LINK(page_fault_linkage, page_fault_handler);
COMMON_INT_LINK(floating_point_error_linkage, floating_point_error_handler);
COMMON_INT_LINK(alignment_check_error_linkage, alignment_check_handler);
COMMON_INT_LINK(machine_check_linkage, machine_check_handler);
COMMON_INT_LINK(floating_point_exception_linkage, floating_point_exception_handler);
COMMON_INT_LINK(reserved_linkage, reserved_handler);



syscall_linkage:  /*BREAKPOINT HERE*/
	PUSHL %esi
	PUSHL %edi
    PUSHL %ebx
	PUSHFL
	/* add negative 1 to eax, to account for starting at index 0*/
	/*check for valid call numbers 1-10 */
	cmpl $0, %eax  
	je invalid_input
	cmpl $10, %eax /* compare with 9 instead */
	ja invalid_input

	PUSH %edx /*EBX - 1st arg */
	PUSH %ecx /* ECX - 2nd Arg */
	PUSH %ebx /*EDX - 3rd Arg */
	
	/*adjust for zero indexing in the jump table */
	subl $1, %eax

	call *function_table(,%eax,4)

	/*remove pushed args from the stack */

	ADDL $12, %esp
	POPFL
	POPL %ebx
	POPL %edi
	POPL %esi
	IRET

invalid_input:
	POPFL
	POPL %ebx
	POPL %edi
	POPL %esi
	movl $-1, %eax
	IRET




function_table:
	.long system_halt, system_execute, system_read, system_write, system_open, system_close, system_getargs, system_vidmap, system_set_handler, system_sigreturn

