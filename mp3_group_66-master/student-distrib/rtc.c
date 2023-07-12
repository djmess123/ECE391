#include "i8259.h"
#include "lib.h"
#include "rtc.h"
#include "syscalls.h"

#define RTC_IS_OPEN         1
#define RTC_IS_CLOSED       0
#define RTC_REG_A           0x8A
#define RTC_REG_B           0x8B
#define RTC_REG_C           0x8C
#define TRUE                1
#define FALSE               0
#define MAX_RTC_FREQ        1024
#define MIN_RTC_FREQ        2

static uint32_t rate; //frequency setting 2^rate to get the frequency
int rtc_test_counter = 0;
int column, row = 0; 
uint32_t rtc_rates[MAX_PROCESSES] = {0};
uint32_t rtc_virtual_counter[MAX_PROCESSES] = {0};
static int rtc_status[MAX_PROCESSES] = {0};

/* Enables the irq for rtc */
/***
 * initilizes the RTC 
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: None
 */
void rtc_init(){
  
    cli();			                // disable interrupts

    enable_irq(RTC_IRQ_NUM); // enable the irq

    //rtc_status = RTC_IS_OPEN;

    outb(0x70, RTC_REG_B);		// select register B, and disable NMI
    char prev = inb(0x71);      	// read the current value of register B
    outb(0x70, RTC_REG_B);		    // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, 0x71);	    // write the previous value ORed with 0x40. This turns on bit 6 of register B
    //sti();
}

/***
 * handles rtc interrupt
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: None
 */
void handle_rtc_interrupt(){
    cli();
    //test_interrupts();
    
    outb(0x0C,RTC_PORT);  //Need to Read from Register C after IRQ 8 so interrupts happen again. 
    inb(RTC_DATA_PORT);

    //rtc_virtual_counter += rtc_virtual_counter % MAX_RTC_FREQ;

    int i = 0;
    for (i = 0; i < MAX_PROCESSES; ++i)
    {
        if (rtc_virtual_counter[i] > 0)
            --rtc_virtual_counter[i];
        if (rtc_virtual_counter[i] <= 0)
            rtc_status[i] = 1;
    }

    //--------------------------
    //TEST CASE
    //--------------------------
    /*
    printf("1");
    column++;
    
    if(column==80){
        row++;
        printf("\n");
        column=0;
    }
    if(row==25){
        row = 0;
        clear();
    }
    
    ++rtc_test_counter;             //for test
    */

    send_eoi(RTC_IRQ_NUM);
    // check the status from the keyboard
    sti();
}

/**
 * Writes to CMOS and RTC ports:
 * INPUT: 8 bit Register to write to, 8 bit data to store in the register
 * OUTPUT: None
 * SIDE EFFECTS: Writes Register to port 0x70, Data to port 0x71
 */

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{   
    int power_of_2_checker = FALSE; //true if the value is a power of 2, false if not
    //null test, if null we failed and return
    if(buf == NULL){
        return -1;
    }
    
    //get the rtc frequency from the deref buffer
    rate = *((uint32_t*)buf);

    //check if power of 2 and not equal to 0, true if power of 2, false if not
    if((rate != 0) && ((rate & (rate - 1)) == 0)){
        power_of_2_checker = TRUE;
    }
    else{
        power_of_2_checker = FALSE;
    }

    //if power of 2 is false return -1 and the frequency was invalid
    if(power_of_2_checker == FALSE){
        return -1;
    }
    
    //virtualize! (based on PID)
    int new_rate = MAX_RTC_FREQ/rate;
    //rtc_virtual_counter[ terminal[cur_term].cur_PCB->Pid ] = new_rate;
    rtc_rates[ terminal[cur_term].cur_PCB->Pid -1 ] = new_rate;
    
    //return 0 on success!
    return 0;
}

/**
 * Reads from CMOS and RTC ports:
 * INPUT: 8 bit Register to read from
 * OUTPUT: 8 bit data stored at that register
 * SIDE EFFECTS: None
 */

int32_t rtc_read(int32_t fd, const void* buf, int32_t nbytes)
{
    // rtc must only return after interrupt has occured.  
    //if the status is 0 that means it is closed and we received an interrupt
    //rtc_status = RTC_IS_OPEN; //set the status to open to wait for interrupt to happen 
    int pid = terminal[cur_term].cur_PCB->Pid - 1;

    rtc_status[ pid ] = 0;      //status as not triggered
    rtc_virtual_counter[pid] = rtc_rates[pid];  //reset counter
    while(1){
        if(rtc_status[ pid ] == 1) { //check if interrupt has occured
            return 0; //return rtc_status; //return 0... (rtc status during interrupt is 0)  rtc_is_closed = 0.   
        }
    }
    
}

/**
 * Reads from CMOS and RTC ports and sets frequency 
 * INPUT: pointer to file
 * OUTPUT: 0 on success
 * SIDE EFFECTS: changes interrupt rate
 */
 int32_t rtc_open(const uint8_t* filename){
    // null pointer test
    if(filename == 0){
        return -1;
    }

    rtc_frequency_change(MAX_RTC_FREQ);
    return 0;
}

/**
 * Reads from CMOS and RTC ports:
 * INPUT: pointer to file
 * OUTPUT: return 0 on success
 * SIDE EFFECTS: changes interrupt rate
 */
int32_t rtc_close(int32_t fd){

    //null test, need valid fd
    if(fd == 0){
        return -1;
    }
    disable_irq(RTC_IRQ_NUM);
    //rtc_status = RTC_IS_CLOSED;
    return 0;
}

/**
 * TODO: sets frequency of the clock    
 * INPUT: frequency 
 * OUTPUT: none
 * SIDE EFFECTS: changes the frequency of the clock
 */
void rtc_frequency_change(uint32_t rtc_freq_variable){
    //initilize variables for log function
    char frequency_check = 0;
    uint32_t freq_var = rtc_freq_variable;
    //see how many times you can divide by 2 (check if a valid power of 2 in write function)
    while(1){
        freq_var = freq_var / 2;
        frequency_check = frequency_check + 1;
        if(freq_var == 1){
            break;
        }
    }
    rate = 15 - frequency_check + 1;

    //if the rate is 1 or 2 OR above 15 (max rate) return because youre moving too fast
    if(rate < 3 || rate > 15){
        return;
    }

    //clear interrupts
    cli();

    outb(RTC_REG_A, RTC_PORT); //write register a to the rtc port
    char prev = inb(RTC_DATA_PORT); // read from the data port
    outb(RTC_REG_A, RTC_PORT); //write to the data port
    char rate_change = (prev&0xF0) | rate;
    outb(rate_change, RTC_DATA_PORT); //Set lower 4 bits of Control register A to desired interrupt rates

    //rtc_status = RTC_IS_OPEN; //set flag

    //set interrupts
    sti();
}

/* test case functions for rtc */

/**
 * Description: helper function to get the variable from this file   
 * INPUT: none
 * OUTPUT: rtc_test_counter (used for testing)
 * SIDE EFFECTS: none
 */
int get_rtc_counter(){
    return rtc_test_counter;
}

/**
 * Description: helper function to set the variable from this file   
 * INPUT: counter (initilizes the variable to the counter usually 0)
 * OUTPUT: none
 * SIDE EFFECTS: sets the counter variable for testing
 */
void set_rtc_counter(int counter){
    rtc_test_counter = counter;
}
