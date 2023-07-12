#include "lib.h"
#include "types.h"
#include "keyboard.h"

#define CHANNEL0    0x40 //port for the channel 0 register
#define PIT_IRQ     0 // channel 0 is connected directly to irq 0 <- OS DEV
#define COM_REG     0x43 // port for the mode and command register (write only, reads ignored)
#define PIT_FQ      1193182 /* pit runs at about 1.193182 MHz <- OS Dev */ 

// sets up the pit controller 
void pit_init();

//uses the handler to get scheduling going for processes
void handle_pit_interrupt();
