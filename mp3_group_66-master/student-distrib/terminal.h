#include "lib.h"
#include "FileSys.h"

#ifndef _TERMINAL_H
#define _TERMINAL_H

#define ONE_KB 1024
#define T_BUF_SIZE 128
#define NUM_TERMINALS 3

#define KEY_BUF_SIZE 128
#define HIST_BUF_SIZE 32

#define OPEN 1
#define CLOSED 0

//video memory 
#define VIDEO       0xB8000
#define TERM_ONE_VIDEO 0xB9000
#define TERM_TWO_VIDEO 0xBA000
#define TERM_THREE_VIDEO 0xBB000

//TODO: implement this structure
typedef struct terminal {
    int terminal_open;
    int shell_flag;
    PCB_t* cur_PCB;
    uint8_t keyboard_buf[T_BUF_SIZE];
    int buf_pos;
    int autofill_location;
    volatile int history_location;
    int current_location;
    int space_count;
    int can_backspace;
    int enter_press; //dont know if I need this
    uint8_t terminal_buf[T_BUF_SIZE];
    int screen_x;
    int screen_y;
    int video_mem;
    int num_processes;
    uint32_t ebp;
    uint32_t esp;
    uint8_t autofill_buf[KEY_BUF_SIZE];
    volatile uint8_t history_buf[HIST_BUF_SIZE][KEY_BUF_SIZE]; //buffer to get recent history

    //save_t ss;  //saved state of the program
}terminal_t;

typedef struct save {
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t flags;
}save_t;


terminal_t terminal[NUM_TERMINALS];

// 1 indexed
volatile int cur_term; // Terminal in which the process is running

int vis_term; //terminal which is visible from the screen

uint8_t terminal_buffer[T_BUF_SIZE];



//initializes terminal
int32_t open_terminal(const uint8_t * filename);

//closes terminal
int32_t close_terminal(int32_t fd);

//reads characters in from keyboard buffer
int32_t read_terminal(int32_t fd, uint8_t * buf, int32_t nbytes);

//Writes characters to screen
int32_t write_terminal(int32_t fd, uint8_t * buf, int32_t nbytes);

//helper: sets which terminal is active
void set_visual_terminal(int terminal_number);

//helper: clears terminal
void clear_terminal_buffer();

//helper: updates cursor position 
void update_cursor(int x, int y);

//Initializes all of the terminals
void terminal_init();

//resets the terminal
void terminal_reset();





#endif /* _TERMINAL_H */
