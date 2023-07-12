/*
 * tab:2
 *
 * search_asm.S - Implementation of Assembly Recursive DFS
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Aamir Hasan
 * Version:       1
 * Creation Date: Fri Aug 30 2020
 * Filename:      search_asm.S
 * History:
 *    AS    1    Fri Aug 30 2020
 *        First written.
 */

.data
  KEY   = 0
  LEFT  = 4
  RIGHT = 8

.text
.global search_asm

# Search ASM (Assembly)
# Searches for an element in a BST
# Declaration is in search.h
#
# Registers:
#   eax - Return Value
# 
search_asm:
  pushl %ebp
  movl %esp, %ebp
#//callee save
  movl $1, %eax
  pushl %ebx
  pushl %edi
  pushl %esi

#  //init
  movl 8(%EBP), %ebx        //load root* into EBX
  movl 12(%EBP), %edi        //load key to ECX
  andl $-1 ,%eax    //Set return reg to -1 by default
#
  //check if NULL
  cmpl  $0, %ebx             //Compares Root against NULL
  je   end_of_func          //jumps to end of function

  //check key vs roots key
  movl (%ebx), %edx        //EDX holds key from root 
  cmpl %edi, %edx        //Compare EDX to key
  je   found

  //not found, less than, greater than?
  cmpl %edi, %edx
  jg  go_left
  jl  go_right


found:
  Andl $0 ,%EAX
end_of_func:
  Popl %esi
  Popl %edi
  Popl %ebx
  leave 
  ret


go_left:
  Pushl %ECX            //caller save ECX, Eflags
  Movl 4(%ebx), %edx        //grabbing left root
#  Pushl %edx            //push args
  Pushl %edi            //push args
 pushl %edx
 Call search_asm        //call search_asm
  Popl    %ECX                //pop caller save
  Jmp end_of_func

go_right:
Pushl %ecx            //caller save ECX, Eflags
  Movl 8(%ebx), %edx        //grabbing right root
#  Pushl %edx            //push args
  Pushl %edi            //push args
 pushl %edx
 Call search_asm        //call search_asm
  Jmp end_of_func

