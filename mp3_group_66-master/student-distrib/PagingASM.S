
.globl loadPageDirectory
.globl enablePaging
.globl flush_tlb


.align 4
/*  puts address of page directory into computer register
    input: arg = word pointer to page directory
    output: none
    side effects: modifies CR3 */
loadPageDirectory:
    pushl %ebp        
    movl %esp, %ebp   
    movl 8(%esp), %eax
    movl %eax, %cr3   
    movl %ebp, %esp   
    popl %ebp         
    ret     

/*  enables paging by modifying a bit in CR0 and cr4 to enable 4mb pages
    input: none
    output: none
    side effects: modifies CR0 and Cr4*/
enablePaging:
    pushl %ebp          
    movl %esp, %ebp     
    movl %cr4, %eax
    orl  $0x10, %eax
    movl %eax, %cr4
    movl %cr0, %eax     
    orl  $0x80000000, %eax
    movl %eax, %cr0    
    leave          
    ret    

/*bookmark weird tlb flush? just puts tlb back?*/

flush_tlb:
    movl	%cr3,%eax
	movl	%eax,%cr3
    ret

