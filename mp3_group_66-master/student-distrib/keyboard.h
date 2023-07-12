#include "lib.h"
//TODO: write file header
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KEYBOARD_IRQ_NUM 1
#define KEYBOARD_DATA_PORT 0x60

extern int enter_press;

// initilizes the keyboard
void keyboard_init();

// handles the keyboard interrupt that come in
void handle_keyboard_interrupt();

// function to print the pressed key to the screen
void key_pressed_handler(unsigned int scancode);

// adds a character to the keyboard buffer
void add_to_keyboard_buffer(uint8_t c);

// clears keyboard and resets keyboard variables
void clear_keyboard_buffer();

void clear_vis_keyboard_buffer();

//the keyboard terminal buffer 
void keyboard_buffer_to_terminal_buffer();

void keyboard_buffer_to_vis_terminal_buffer();

//puts the keyboard buffer into the history buffer
void keyboard_buffer_to_history_buffer();

//puts the history we need to show to the screen
void history_to_screen();

//puts the history buffer into the keyboard buffer
void history_buffer_to_keyboard_buffer();

//updates the current location pointer in the array so we know what to display
void update_current_location(int8_t scan_code);

//checks the validity of the history location.  circulates
void history_count_check();

//delets the current text from the keyboard buffer
void delete_current_text(int loc);

//clears the history buffer at a specific location
void clear_history_buffer_at_location(int loc);

//clears the autofill buffer
void clear_autofill_buffer();

//clears the history buffer for the visible terminal
void clear_history_buffer();

//checks if there is a match for autofill
uint8_t check_autofill_match();

//stores the history buffer to the auto fill buffer
void history_buffer_to_autofill_buffer();


#endif /* _KEYBOARD_H */ 
