/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/***
 * initlizes the i8259 pic
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: sets up the ports for the pic
 */
void i8259_init(void) {
    
    outb(0xff, 0x21);
    outb(0xff, 0xA1);

	outb(ICW1, MASTER_8259_PORT);  // starts the initialization sequence (in cascade mode)
	outb(ICW2_MASTER, MASTER_8259_DATA);
	outb(ICW3_MASTER, MASTER_8259_DATA);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(ICW4, MASTER_8259_DATA);
 
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_DATA);
	outb(ICW3_SLAVE, SLAVE_8259_DATA);   // restore saved masks.
	outb(ICW4, SLAVE_8259_DATA);

    enable_irq(2);
}

/* Enable (unmask) the specified IRQ */

/***
 * unmasks at the specific irq
 * INPUT: irq_num, the number interrupt requested
 * OUTPUT: None
 * SIDE EFFECTS: unmasks the irq number enables it at the correct ports
 */
void enable_irq(uint32_t irq_num) {
    
    int pic_mask;
 
    if(irq_num < PIN_NUMBER) {
        pic_mask = 1 << irq_num;
        pic_mask = ~pic_mask;
        master_mask = pic_mask & master_mask;
        outb(master_mask, MASTER_8259_PORT + 1);
    } 
    else {
        irq_num -= PIN_NUMBER;
        pic_mask = 1 << irq_num;
        pic_mask = ~pic_mask;
        slave_mask = pic_mask & slave_mask;
        outb(slave_mask, SLAVE_8259_PORT + 1); 
    }
}

/* Disable (mask) the specified IRQ */

/***
 * unmasks at the specific irq
 * INPUT: irq_num, the number interrupt requested
 * OUTPUT: None
 * SIDE EFFECTS:  unmasks the irq number disables it at the correct ports
 */
void disable_irq(uint32_t irq_num) {

    int pic_mask;

    if(irq_num < PIN_NUMBER) {
        pic_mask = 1 << irq_num;
        master_mask = pic_mask | master_mask;
        outb(slave_mask, SLAVE_8259_PORT + 1);
    } else {
        irq_num -= PIN_NUMBER;
        pic_mask = 1 << irq_num;
        slave_mask = pic_mask | slave_mask;
        outb(master_mask, MASTER_8259_DATA);
    }

}

/* Send end-of-interrupt signal for the specified IRQ */

/***
 *sends end of interrupt single at the specified irq number
 * INPUT: irq_num, the number interrupt requested
 * OUTPUT: None
 * SIDE EFFECTS: ends the interrupt
 */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= PIN_NUMBER){
        outb(EOI | (irq_num - PIN_NUMBER), SLAVE_8259_PORT);
        outb(EOI | 2,  MASTER_8259_PORT);
    }
    else{
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
}
