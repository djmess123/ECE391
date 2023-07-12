#include "terminal.h"
#include "i8259.h"
#include "lib.h"
#include "keyboard.h"
#include "Paging.h"


#define T_BUF_SIZE 128
#define VGA_WIDTH 80



/***
 * initializes terminal
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: initializes terminal variables
 */
void terminal_init(){
    int i;
    //sets atributes for each terminal
    for(i = 0; i < NUM_TERMINALS; i++){
        cur_term = i; //makes it so helper functions in other files can be used
        terminal[i].terminal_open = OPEN;
        terminal[i].cur_PCB = (PCB_t*)NULL;
        terminal[i].num_processes = 0;
        terminal[i].shell_flag = CLOSED;
        //figure out how to initialize keyboard and terminal buffer  (buf_pos and enter_press)
        clear_terminal_buffer();
        clear_keyboard_buffer();
        //clears video memory and resets screen_x and screen_y (from lib.c)
        clear();
    }
    terminal[0].video_mem = TERM_ONE_VIDEO;
    terminal[1].video_mem = TERM_TWO_VIDEO;
    terminal[2].video_mem = TERM_THREE_VIDEO;
    vis_term = 0;
    cur_term = 0;
}

/***
 * resets the terminal for use
 * INPUT: none
 * OUTPUT: none
 * SIDE EFFECTS: resets the buffers for use
 * */
void terminal_reset(){
    clear_terminal_buffer();
    clear_keyboard_buffer();
}


/***
 * opens terminal interface
 * INPUT: filename
 * OUTPUT:returns 0 if sucsess
 * SIDE EFECTS: changes terminal_open flag
 */
int open_terminal(const uint8_t * filename){
    if(terminal[cur_term].terminal_open == OPEN){
        return 0; 
    }
    //TODO: add aditional logic if necessary
    clear_terminal_buffer();
    terminal[cur_term].terminal_open = OPEN;
    return 0;
}

/***
 * closes terminal interface
 * INPUT: filename
 * OUTPUT:returns 0 if sucsess
 * SIDE EFECTS: changes terminal_open flag
 */
int32_t close_terminal(int32_t fd){
    if(terminal[cur_term].terminal_open == CLOSED){
        return 0;
    }
    //TODO: add aditional logic if necessary
    clear_terminal_buffer();
    terminal[cur_term].terminal_open = CLOSED;
    return 0;
}

/***
 * reads stuff from the terminal
 * INPUT: file directory, buffer to read from, # of bytes
 * OUTPUT: number of bytes read
 * EFFECTS: copies passed in buffer to terminal buffer
 */
int32_t read_terminal(int32_t fd, uint8_t * buf, int32_t nbytes){
    int32_t i;
    int32_t iterations;
    //if(terminal_open == CLOSED) return 0;
    if(buf == NULL) return 0;
    //clear_terminal_buffer();


    // Makes the terminal hang until enter has been pressed
    //TODO: @nick this might not be cur_term with scheduling
    terminal[cur_term].enter_press = 0; 
    while(1) {
        //sti();
        if (terminal[cur_term].enter_press == 1)
            break;
        //eiwowreo
        //cli();
    }
    terminal[cur_term].enter_press = 0;

    iterations = T_BUF_SIZE;

    if(iterations > nbytes){
        iterations = nbytes;
    }

    for(i = 0; i < iterations; i++){
        buf[i] = terminal[cur_term].terminal_buf[i];
        if(buf[i] == '\n'){
            
            // sti();
            return i;
        }
    }
    //sti();
    terminal[cur_term].enter_press = 0;

    return T_BUF_SIZE;
}

/***
 * writes stuff from the terminal
 * INPUT: file directory, buffer to write from, # of bytes
 * OUTPUT: number of bytes read
 * EFFECTS: writes buffer to terminal
 */
int32_t write_terminal(int32_t fd, uint8_t * buf, int32_t nbytes){
    int32_t i;
    //check file directory
    //if(terminal_open == CLOSED) return 0;
    if(fd != 1) return 0;
    for(i = 0; i < (nbytes); i++){
        putc(buf[i]);
    }
    return nbytes;
}

/***
 * clears the terminal buffer
 * INPUT: None
 * OUTPUT: None
 * EFFECTS: resets terminal buffer variables and clears terminal buffer
 */
void clear_terminal_buffer(){
    int i;
    for(i= 0; i < T_BUF_SIZE; i++){
        terminal[cur_term].terminal_buf[i] = 0x00;
    }
    return;
}


/***
 * Updates the position of the cursor
 * INPUT: int x, int y
 * OUTPUT: None
 * EFFECTS: Puts the cursor at the location specified by x and y
 */
void update_cursor(int x,int y)
{
    //TODO: @luca can you comment this?
	uint16_t pos = y * VGA_WIDTH + x; //Get cursor position
 
	outb(0x0F,0x3D4);                       //Set Cursor Registers
	outb((uint8_t) (pos & 0xFF),0x3D5);
	outb(0x0E,0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF),0x3D5);
}

/***
 * Updates the which terminal is current (1 INDEXED!!!)
 * Maybe change to set visual terminal
 * INPUT: index of terminal you wanna make visual
 * OUTPUT: None
 * EFFECTS: Puts the cursor at the location specified by x and y
 */
void set_visual_terminal(int terminal_number){
    if((terminal_number <= 0)||(terminal_number > NUM_TERMINALS)){
        //invalid terminal number
        return;
    }
    //set_vid_address(OUTPUT);
    save_vidmem(cur_term);
    //copy video memory from Video buffer to visible terminal memory
    memcpy((void*)terminal[vis_term].video_mem,(void*)VIDEO, 4*ONE_KB);
    //copy video memory of terminal_num to the primary video memory
    memcpy((void*)VIDEO,(void*)terminal[terminal_number-1].video_mem,4*ONE_KB);
    //(terminal_num is one indexed)
    vis_term = terminal_number - 1;

    update_cursor(terminal[vis_term].screen_x,terminal[vis_term].screen_y);
    
}

