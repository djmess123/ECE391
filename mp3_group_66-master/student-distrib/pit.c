#include "pit.h"
#include "scheduler.h"
#include "i8259.h"
#include "lib.h"

/***
 * sets up the PIT and moves the frequencies into the timer to generate interrupts
 * INPUT: none
 * OUTPUT: none
 * SIDE EFFECTS: sets frequency for the PIT and moves values into ports
 * */
void pit_init(){
    /* os dev */

    cli();

    enable_irq(PIT_IRQ);

    int32_t freq = PIT_FQ / 100; //set interrupt to every 10 ms

    outb(0x36, COM_REG); // set the PIT to timer interrupts using the 0x36 port on the PIC

    outb(freq&0xFF, CHANNEL0); //set the low bytes
    outb((freq&0xFF00)>>8, CHANNEL0); //set the high bytes

    //sti();
}

/***
 * every time we get an interrupt call the scheduler to run the processes
 * INPUT: none
 * OUTPUT: none
 * SIDE EFFECTS: calls scheduler to get processes running
 * */
void handle_pit_interrupt(){
    //cli(), sti() and end of Interupt done in scheduler
    scheduler();


    //sti();
}

