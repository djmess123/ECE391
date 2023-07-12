
.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

        FAIL = -1
        SUCC = 0

        MIN_VRAM_LOCATION = 0
        MAX_VRAM_LOCATION = 1999

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

jump_table:
.long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret




# void mp1_rtc_tasklet(unsigned long);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: 
# Clobbers EDX
# esi = linked list pointer
# edi = old status
# ebx = new length
mp1_rtc_tasklet:
    pushl   %ebp                                    #callee save
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %edi
    pushl   %esi
    
    movl    mp1_list_head, %esi                     #init loop
    andl    $0, %edi
    andl    $0, %ebx

    LIST_LOOP_START:                                #traverse linked list 0-N
        cmpl    $0, %esi                             #Check if end node (NULL)
        je      LIST_LOOP_BREAK

        decw    COUNTDOWN(%esi,1)                   #update countdown
        cmpw    $0, COUNTDOWN(%esi,1)          
        jne      LIST_LOOP_CONTINUE                 #is new countdown == 0?
            movw	STATUS(%esi,1), %di             #save old status for later
            xorw	$1, STATUS(%esi,1)              #flip status in mem

            andl    $0, %eax
            movb    ON_CHAR(%esi,%edi,1), %cl       #poke location with new char
            movw	LOCATION(%esi,1), %ax
            shll	$1, %eax                        #multiply location by 2 (2 bytes per char)
            call    mp1_poke
            movw    ON_LENGTH(%esi,%edi,2), %bx     #update countdown with new status's length
            movw    %bx, COUNTDOWN(%esi,1)
        LIST_LOOP_CONTINUE:
        movl    NEXT(%esi,1), %esi                  #make current struct = old's next
        jmp     LIST_LOOP_START
    LIST_LOOP_BREAK:
    popl    %esi                                    #callee return
    popl    %edi
    popl    %ebx
    leave
	ret




# Dispatcher
#
# Interface: c-calling convention
#    Inputs: 
#               8  long arg
#               12 long cmd
#   Outputs:    %eax = 0 for success, -1 for failure
#   Registers: 
#               Clobbers EDX
#               edi = cmd

mp1_ioctl:
    pushl   %ebp                    #callee save
    movl    %esp,   %ebp
    pushl   %ebx
    pushl   %edi
    pushl   %esi
    movl    12(%ebp), %edi          #pull cmd off stack
    jmp     *jump_table(,%edi,4)    #indirect jump to jump_table + cmd*4(bytes)
mp1_ioctl_mem_fail: 
    pushl   %ebx                    #there was an error, DEALLOCATE MEM, THEN return fail
    call    mp1_free
    addl    $4, %esp                #pop 1 off stack
mp1_ioctl_return_fail:
    movl	$FAIL, %eax             #return fail condition
    popl     %esi
    popl    %edi
    popl    %ebx
    leave
    ret
mp1_ioctl_return_success:
    movl    $SUCC, %eax             #return success condition
    popl    %esi
    popl    %edi
    popl    %ebx
    leave
    ret




#   ADD a single blinking location
#
#   Interface:  jump table from Dispatcher, same IO
#   Registers: 
#               ebx: malloc pointer
#               edi: temporary var for comparisons
mp1_ioctl_add:
    #NULL Guard argument
    movl	8(%ebp), %edi          #use %edi to hold ioctl arg temporarily
    cmpl	$0, %edi               
    je      mp1_ioctl_return_fail
    
    #mp1_malloc struct   (returns NULL if error, remember to free errored allocations)
    pushl   $STRUCT_SIZE
    call    mp1_malloc
    addl    $4, %esp
    
    #check malloc (success != 0, fail = 0)
    cmpl	$0, %eax                
    je      mp1_ioctl_return_fail
    movl    %eax, %ebx             #put malloc address in non-volitile reg
    
    #mp1_copy_from_user struct to allocated mem
    pushl   $STRUCT_SIZE
    movl	8(%ebp), %edi          #use %edi to hold ioctl arg temporarily
    pushl   %edi                   #ioctl arg on stack   from
    pushl   %ebx                   #malloc address       to
    call    mp1_copy_from_user
    addl    $12, %esp              #pop 3 off stack
    
    #check copy_to_user (success = 0, fail != 0)
    cmpl	$0, %eax                
    jne     mp1_ioctl_mem_fail
    
    #range check LOCATION               use %edi to hold location temporarily
    movw    LOCATION(%ebx,1), %di
    cmpw	$MIN_VRAM_LOCATION, %di                
    jl      mp1_ioctl_mem_fail
    cmpw	$MAX_VRAM_LOCATION, %di                
    jg      mp1_ioctl_mem_fail
    
    #set countdown = on_length          use %edi to hold on_length temporarily
    movw	ON_LENGTH(%ebx,1), %di
    movw	%di, COUNTDOWN(%ebx,1)
    
    #set status = 1
    movw	$1, STATUS(%ebx,1)
    
    #insert node at head pointer        use %edi to hold list_head temporarily
    movl	mp1_list_head, %edi
    movl	%edi, NEXT(%ebx,1)
    movl	%ebx, mp1_list_head
    
    #mp1_poke
    andl    $0, %eax
    movb    ON_CHAR(%ebx,1), %cl       #poke location with new char
    movw	LOCATION(%ebx,1), %ax
    shll	$1, %eax                   #multiply location by 2 (2 bytes per char)
    call    mp1_poke
    
    #return 0 on success
    jmp     mp1_ioctl_return_success



#   REMOVE a blinking location
#
#   Interface:  jump table from Dispatcher, same IO
#   Registers: 
#               ebx: node pointer (if found)
#               esi: node parent pointer (if found)
#               edi: target location
mp1_ioctl_remove:
    #mp1_list_search for location
    movl	8(%ebp), %edi          #use %edi to hold target location
    andl    $0, %ebx
    andl    $0, %esi
    
    #Search list
    subl    $8, %esp                   # 2 blank args
    pushl   %edi                       #target
    call    mp1_list_search
    addl    $4, %esp                   #trash target
    popl    %ebx                       #grab node (if found, else == NULL)
    popl    %esi                       #grab parent node (if found, else == NULL)
    
    #NULL guard
    cmpl    $0, %eax                   
    jne     mp1_ioctl_return_fail
    cmpl    $0, %ebx
    je      mp1_ioctl_return_fail
    cmpl    $0, %esi
    je      mp1_ioctl_return_fail
    #found valid node!!

    #is node head?
    cmpl    %ebx, mp1_list_head                
    jne     REMOVE_NOT_HEAD
    REMOVE_HEAD:
        movl    NEXT(%ebx,1), %edx              #make parent->next point to target->next
        movl    %edx, mp1_list_head             #make head point to target->next
        jmp     REMOVE_FREE_MALLOC
    REMOVE_NOT_HEAD:
        movl    NEXT(%ebx,1), %edx              #make parent->next point to target->next
        movl    %edx, NEXT(%esi,1)
    
    REMOVE_FREE_MALLOC:
    pushl   %ebx
    call    mp1_free
    addl    $4, %esp                            #pop 1 off stack
    jmp     mp1_ioctl_return_success



#   FIND a blinking location
#
#   Interface:  jump table from Dispatcher, same IO
#   Registers: 
#               ebx: malloc pointer
#               esi: node pointer (if found)
#               edi: temp
mp1_ioctl_find:
#validate input struct --------------------------------------------------------------------
    #NULL Guard argument
    movl	8(%ebp), %edi          #use %edi to hold ioctl arg temporarily
    cmpl	$0, %edi               
    je      mp1_ioctl_return_fail
    #mp1_malloc struct   (returns NULL if error, remember to free errored allocations)
    pushl   $STRUCT_SIZE
    call    mp1_malloc
    addl    $4, %esp               #pop 1 off stack
    #check malloc (success != 0, fail = 0)
    cmpl	$0, %eax                
    je      mp1_ioctl_return_fail
    movl    %eax, %ebx             #put malloc address in non-volitile reg
    #mp1_copy_from_user struct to allocated mem
    pushl   $STRUCT_SIZE
    movl	8(%ebp), %edi          #use %edi to hold ioctl arg temporarily
    pushl   %edi                   #ioctl arg on stack   from
    pushl   %ebx                   #malloc address       to
    call    mp1_copy_from_user
    addl    $12, %esp
    #check copy_to_user (success = 0, fail != 0)
    cmpl	$0, %eax                
    jne     mp1_ioctl_mem_fail
    #range check LOCATION               use %edi to hold location temporarily
    andl    $0, %edi
    movw    LOCATION(%ebx,1), %di
    cmpw	$MIN_VRAM_LOCATION, %di                
    jl      mp1_ioctl_mem_fail
    cmpw	$MAX_VRAM_LOCATION, %di                
    jg      mp1_ioctl_mem_fail


#search linked list for a match with location ----------------------------------------------
    #subl    $8, %esp                # 2 blank args
    pushl	$0
	pushl	$0
    pushl   %edi                    #target location
    call    mp1_list_search
    addl    $4, %esp                #trash target
    popl    %esi                    #grab node (if found, else == NULL)
    addl    $4, %esp                #trash parent
    
    #NULL guard
    cmpl    $0, %eax                #check ret val for error    
    jne     mp1_ioctl_mem_fail
    cmpl    $0, %esi                #null guard node
    je      mp1_ioctl_mem_fail
    #found valid node!!

#found location -> overwrite user input (mp1_copy_to_user) ------------------------------------
    pushl   $STRUCT_SIZE
    pushl   %esi                      #found node   from
    pushl   8(%ebp)                   #user arg     to
    call    mp1_copy_to_user
    addl    $12, %esp                 #pop 3 off stack
    #check copy_to_user (success = 0, fail != 0)  
    cmpl	$0, %eax                
    jne     mp1_ioctl_mem_fail
    #return success!
    pushl   %ebx
    call    mp1_free
    addl    $4, %esp                  #pop 1 off stack
    jmp mp1_ioctl_return_success



#   synchronize a blinking location with another
#
#   Interface:  jump table from Dispatcher, same IO
#   Registers: 
#               ebx: node pointer 1 (if found)
#               esi: node pointer 2 (if found)
#               edi: target location / swap register
mp1_ioctl_sync:    
    #search list for location 1 (upper 2 bytes)
    movw	10(%ebp), %di          #use %edi to hold target location
    andl    $0, %ebx
    andl    $0, %esi
    #Search list
    #subl    $8, %esp                # 2 blank args
    pushl	$0
	pushl	$0
    pushl   %edi                       #target
    call    mp1_list_search
    addl    $4, %esp                   #trash target
    popl    %ebx                       #grab node (if found, else == NULL)
    addl    $4, %esp                   #trash parent
    #NULL guard
    cmpl    $0, %eax                   
    jne     mp1_ioctl_return_fail
    cmpl    $0, %ebx
    je      mp1_ioctl_return_fail
    #found valid node 1!!

    #Search list for location 2 (lower 2 bytes)
    movw	8(%ebp), %di          #use %edi to hold target location
    #Search list
    #subl    $8, %esp                # 2 blank args
    pushl	$0
	pushl	$0
    pushl   %edi                       #target
    call    mp1_list_search
    addl    $4, %esp                   #trash target
    popl    %esi                       #grab node (if found, else == NULL)
    addl    $4, %esp                   #trash parent
    #NULL guard
    cmpl    $0, %eax                   
    jne     mp1_ioctl_return_fail
    cmpl    $0, %esi
    je      mp1_ioctl_return_fail
    #found valid node 2!!

    #arg 2's on_length, off_length, coundown, status is overwritten with arg1's
    movw ON_LENGTH(%ebx,1), %di
    movw %di, ON_LENGTH(%esi,1)

    movw OFF_LENGTH(%ebx,1), %di
    movw %di, OFF_LENGTH(%esi,1)

    movw COUNTDOWN(%ebx,1), %di
    movw %di, COUNTDOWN(%esi,1)

    movw STATUS(%ebx,1), %di
    movw %di, STATUS(%esi,1)

    #poke 2nd location, using its new status bit
    andl    $0, %eax
    movb    ON_CHAR(%esi,1), %cl       #poke location with new char
    movw	LOCATION(%esi,1), %ax
    shll	$1, %eax                   #multiply location by 2 (2 bytes per char)
    call    mp1_poke
    jmp mp1_ioctl_return_success
	





# Search helper function
#
# Interface: c-calling convention
#    Inputs: 
#               8  long target_location
#               12 long * found_location    (= NULL if not found)
#               16 long * found_parent      (= NULL if not found)
#   Outputs:    
#               long error_code (= -1 if not found, 0 if found)
#   Registers: 
#               edi:    previous pointer
#               esi:    current node pointer
#               ebx:    target location
mp1_list_search:
    pushl %ebp              #callee save
    movl  %esp, %ebp
    pushl %ebx
    pushl %edi
    pushl %esi

    movl    mp1_list_head, %esi             #init loop
    leal    mp1_list_head, %edi
    andl    $0, %ebx             
    movw    8(%ebp,1), %bx                  #grab target_loc into ebx
    movl    $FAIL, %eax                           

    SEARCH_LOOP_START:                      #traverse linked list
        cmpl    $0, %esi                    #Check if end node (NULL)
        je      SEARCH_LOOP_BREAK

        cmpw    %bx, LOCATION(%esi,1)                  
        jne     SEARCH_LOOP_CONTINUE        #is current location == target location?
        
        movl    %esi, 12(%ebp,1)            #FOUND! set return values on stack arg 2 
        movl    %edi, 16(%ebp,1)            #and arg 3
        movl    $SUCC, %eax 
        jmp     SEARCH_LOOP_BREAK
    
    SEARCH_LOOP_CONTINUE:
        movl    %esi, %edi                  #prev = curr
        movl    NEXT(%esi,1), %esi          #curr = next
        jmp     SEARCH_LOOP_START
    
    SEARCH_LOOP_BREAK:

    popl %esi
    popl %edi
    popl %ebx
    leave
    ret



.end
