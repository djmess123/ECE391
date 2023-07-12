//TODO: write file header
#ifndef _RTC_H
#define _RTC_H

#define RTC_IRQ_NUM 8
#define RTC_PORT 0x70                       //PORTS from OPENCV
#define RTC_DATA_PORT 0x71 

// Initilizes the rtc
void rtc_init();

// Handles the rtc interrupt that come in
void handle_rtc_interrupt();

//access registers on the CMOS and RTC chips
/**
 * Writes to CMOS and RTC ports:
 * INPUT: 8 bit Register to write to, 8 bit data to store in the register
 * OUTPUT: None
 * SIDE EFFECTS: Writes Register to port 0x70, Data to port 0x71
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/**
 * Reads from CMOS and RTC ports:
 * INPUT: 8 bit Register to read from
 * OUTPUT: 8 bit data stored at that register
 * SIDE EFFECTS: None
 */
int32_t rtc_read(int32_t fd, const void* buf, int32_t nbytes);

/**
 * Reads from CMOS and RTC ports:
 * INPUT: pointer to file
 * OUTPUT: none
 * SIDE EFFECTS: changes interrupt rate
 */
int32_t rtc_close(int32_t fd);

/**
 * Reads from CMOS and RTC ports and sets frequency 
 * INPUT: pointer to file
 * OUTPUT: 0 on success
 * SIDE EFFECTS: changes interrupt rate
 */
int32_t rtc_open(const uint8_t* filename);

/**
 * TODO: sets frequency of the clock    
 * INPUT: frequency 
 * OUTPUT: none
 * SIDE EFFECTS: changes the frequency of the clock
 */
void rtc_frequency_change(uint32_t rtc_freq_variable);

// virtualization for RTC TODO
void rtc_virtualization();

//getter function for the tests
int get_rtc_counter();

//setter function for the tests
void set_rtc_counter(int counter);

#endif /* _RTC_H */ 
