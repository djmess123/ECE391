#include "scheduler.h"
#include "syscalls.h"
#include "terminal.h"
#include "Paging.h"

#define PIT_IRQ 0

//debug text macro
#define PR(fmt, ...) \
            do { if (DBGTXT_SCH) printf(fmt, ##__VA_ARGS__); } while (0)


uint32_t pit_flag = 1;

/* scheduler()*/
/***
 * helper function of PIT, switches context between terminals
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: swaps kernal stacks, virtual memory, paging, processes, all registers are affected, but saved
 */
void scheduler(){
    //this is called when you are about to pause one terminal and launch the next
    
    //check if its the first time the PIT has been called
        //if so, skip saving and go straight to launch shell

    int last_term = cur_term;
    cur_term = (cur_term + 1) % NUM_TERMINALS;

    PR("cur_term: %d  next_term: %d\n", cur_term, (cur_term + 1) % NUM_TERMINALS);
    
    //launching base shells at system start
    if (terminal[cur_term].shell_flag==CLOSED)
    {
        PR("Very First Shell\n");
        //run once
        pit_flag = 0;
        terminal[cur_term].shell_flag = OPEN;

        //save
        save_vidmem(cur_term);
        register uint32_t ebp asm("%ebp");
        terminal[last_term].ebp = ebp;    
        send_eoi(PIT_IRQ);
        
        system_execute((uint8_t*)"shell");
    }
    
    

    //====================================SAVE==========================================

    //Save EBP and other stuff for old process that is not saved by call to interrupt
    register uint32_t esp asm("%esp");
    register uint32_t ebp asm("%ebp");
    terminal[last_term].ebp = ebp;      

    PR("SAVE ebp: %x esp: %x", ebp, esp);

    //====================================LOAD==========================================
    
    //if no pcb has been initialized then launch the shell on the terminal
    if(terminal[cur_term].cur_PCB == NULL)
    {
        //not used?
        PR("---New Base Shell\n");
        send_eoi(PIT_IRQ);
        //execute sets up paging, vid mem, flushes tlb
        save_vidmem(cur_term);
        system_execute((uint8_t*)"shell");
    }

    //remap paging to new terminal
    setup_paging(PAGE_ADDR(terminal[cur_term].cur_PCB->Pid));

    //swap out video memory and remap
    save_vidmem(cur_term);
    set_vidmem(cur_term); 
    save_vidmem(cur_term);

    //flush TLB
    flush_tlb();

    //free up the PIT
    send_eoi(PIT_IRQ);
    
    //update kernal stack location
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KSTACK_ADDR(terminal[cur_term].cur_PCB->Pid);
    //esp = terminal[next_term].esp;
    ebp = terminal[cur_term].ebp;

    //return should just work, see paragraph above
    return;
    


}




