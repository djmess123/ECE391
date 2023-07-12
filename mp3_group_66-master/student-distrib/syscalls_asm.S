/***
 * switches context from kernal to user
 * INPUT: None
 * OUTPUT: none
 * EFFECTS: changes privlage level, swaps to run program in user level
 * REGISTERS: ebx = eip
 */
.GLOBL jump_usermode
jump_usermode:
    movl 4(%esp),%ebx

    pushl $0x02B            #SS    stack segment  == user_data_segment
    pushl $0x083FFFFC       #ESP   stack pointer  == user_stack_pointer

    pushfl  
    popl  %eax
    orl   $0x200, %eax      #interrupt enable flag   bookmark
    pushl %eax    

    pushl $0x0023           #user_cs    code segment
    pushl (%ebx)            #eip        instruction pointer

    iret                    
