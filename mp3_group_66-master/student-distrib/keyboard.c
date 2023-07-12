#include "i8259.h"
#include "lib.h"
#include "keyboard.h"
#include "terminal.h"
#include "syscalls.h"
#include "interrupts.h"

int shift = 0;
int ctrl = 0;
int alt = 0;
int up_arrow = 0;
int down_arrow = 0;
int tab_pressed = 0;
//int can_backspace = 0;
// int current_location = -1;
// int history_location = 0;
// int autofill_location = 0;
int up_press_count = 0;
int down_press_count = 0;




//Key definitions
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define LEFT_SHIFT_RELEASE 0xAA
#define RIGHT_SHIFT_RELEASE 0xB6
#define LEFT_CONTROL 0x1D
#define RIGHT_CONTROL 0xE0
#define LEFT_CONTROL_RELEASE 0x9D
#define RIGHT_CONTROL_RELEASE 0x9D
#define L_PRESS 0x26
#define LEFT_ALT 0x38 
#define LEFT_ALT_RELEASE 0xB8
#define UP_ARROW 0x48
#define UP_ARROW_RELEASE 0xC8
#define DOWN_ARROW 0x50
#define DOWN_ARROW_RELEASE 0xD0
#define ENTER_KEY 0x1C
#define ENTER_KEY_RELEASE 0x9C
#define SPACE_KEY 0x39
#define TAB_KEY 0x0F
#define TAB_KEY_RELEASE 0x8F
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D
#define ON 1
#define OFF 0



static uint32_t keyboard_map[87] = {0x00, 0x00,'1','2','3','4','5','6','7','8','9','0','-','=', //no scan code for 0x00, escape key
                                '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']',
                                '\n',0x00,'a','s','d','f','g','h','j','k','l',';','\'','`', //left control
                                0x00, 0x00,'z','x','c','v','b','n','m',',','.','/',
                                0x00,'*', 0x00,' ',0x00, //right shift, left alt, capslock
                                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //f1-f9
                                0x00,0x00,'7','8','9','-','4','5','6','+','1','2','3','0','.',0x00}; //num lock, scroll lock, f11

static uint32_t keyboard_map_shift[87] = {0x00, 0x00,'!','@','#','$','%','^','7','*','(',')','_','+', //no scan code for 0x00, escape key
                                '\b','\t','Q','W','E','R','T','Y','U','I','O','P','[',']',
                                '\n',0x00,'A','S','D','F','G','H','J','K','L',';','\'','`', //left control
                                0x00, 0x00,'Z','X','C','V','B','N','M',',','.','/',
                                0x00,'*', 0x00,' ',0x00, //right shift, left alt, capslock
                                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //f1-f9
                                0x00,0x00,'7','8','9','-','4','5','6','+','1','2','3','0','.',0x00};

static char* executable_list[] = {"sigtest", "shell", "grep", "syserr", "rtc", "fish", "counter", "pingpong",
                                    "cat", "frame0.txt", "verylargetextwithverylongname.tx", "ls", "testprint",
                                    "created.txt", "frame1.txt", "hello", "exit", "ELF"};

int enter_press = 0;
int enter_down = 0;
int autofill_match_count = 0;

/* Enables the irq for keyboard */
void keyboard_init(){
    clear_terminal_buffer();
    clear_keyboard_buffer();
    clear_history_buffer();
    clear_autofill_buffer();
    enable_irq(KEYBOARD_IRQ_NUM); //TODO: check if IRQ 1 is the default keyboard interrupt pin (note from keith: looks to be one)
}

/***
 * handles keyboard interrupt
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: None
 */
void handle_keyboard_interrupt(){
    int saved_term;
    cli(); //pause interrupts
    //termporarily set cur terminal to visual terminal
    saved_term = cur_term;
    cur_term = vis_term;
    //set_vid_address(OUTPUT);
    unsigned char input;
    input = inb(KEYBOARD_DATA_PORT);

    //set_vidmem(vis_term);

    save_vidmem(vis_term);

    //call the handler function if there is a key pressed
    if(input > 0){
        key_pressed_handler(input);
    }
    //restore the current terminal
    cur_term=saved_term;

    //save_vidmem(cur_term);
    send_eoi(KEYBOARD_IRQ_NUM);
    // check the status from the keyboard
    //cur_term = saved_term
    sti(); //resume interupts
}


/***
 * handles printing pressed key to screen
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: prints character from keyboard IO to the terminal
 */
void key_pressed_handler(unsigned int scan_code){
    // Marks shift flag
    if((scan_code == LEFT_SHIFT) || (scan_code == RIGHT_SHIFT)){ //shift pressed
        shift = ON;
        return;
    }
    //unmarks shift flag 
    if((scan_code == LEFT_SHIFT_RELEASE) || (scan_code == RIGHT_SHIFT_RELEASE)){
        shift = OFF;
        return;
    }
    //mark ctrl flag
    if((scan_code == RIGHT_CONTROL) || (scan_code == LEFT_CONTROL)){
        ctrl = ON;
    }
    //unmark ctrl
    if((scan_code == RIGHT_CONTROL_RELEASE) || (scan_code == LEFT_CONTROL_RELEASE)){
        ctrl = OFF;
    }
    //mark alt
    if((scan_code == LEFT_ALT)){
        alt = ON;
    }
    //unmark alt
    if((scan_code == LEFT_ALT_RELEASE)){
        alt = OFF;
    }
    
    //mark and unmark up arrow
    if(scan_code == UP_ARROW){
        up_arrow = ON;
    }
    else if(scan_code == UP_ARROW_RELEASE){
        up_arrow = OFF;
    }

    //mark and unmark down arrow
    if(scan_code == DOWN_ARROW){
        down_arrow = ON;
    }
    else if(scan_code == DOWN_ARROW_RELEASE){
        down_arrow = OFF;
    }

    //up arrow count check
    if(terminal[vis_term].current_location < -1){
        terminal[vis_term].current_location = -1;
    }

    //mark and unmark the enter key
    if(scan_code == ENTER_KEY){
        enter_down = ON;
    }
    
    if(scan_code == ENTER_KEY_RELEASE){
        enter_down = OFF;
    }

    //mark and unmark the enter key
    if(scan_code == TAB_KEY){
        tab_pressed = ON;
    }
    
    if(scan_code == TAB_KEY_RELEASE){
        tab_pressed = OFF;
    }

    if(scan_code == SPACE_KEY){
        clear_autofill_buffer();
        terminal[vis_term].autofill_location = -1;
        terminal[vis_term].space_count++;
    }


    //TODO: implement terminal switching here
    if(alt == ON){
        switch(scan_code){
            case F1:
                //printf("switch terminal 1");
                set_visual_terminal(1);
                return;
                break;
            case F2:
                //printf("switch terminal 2");
                set_visual_terminal(2);
                return;
                break;
            case F3:
                //printf("switch to terminal 3");
                set_visual_terminal(3);
                return;
                break;
        }
    }


    if((scan_code == L_PRESS) && (ctrl == ON)){
        clear_keyboard_buffer();
        //clear screen 
        clear();
        return;
    }

    if((scan_code == 0x2E) && (ctrl == ON)){
        printf("\nHalting Process.\n");
        //halt_flag = 1;
        send_eoi(KEYBOARD_IRQ_NUM);
        terminal_reset();
        terminal[vis_term].autofill_location = 0;
        system_halt(-1);
    }

    // return if the scan code is out of range (87 is start of release codes)
    if(scan_code > 87){
        return;
    }

    //check if the up arrow key is pressed
    if(up_arrow && (terminal[vis_term].current_location >= 0)){
        //check the locations
        if(terminal[vis_term].current_location == terminal[vis_term].history_location){
            terminal[vis_term].current_location = terminal[vis_term].history_location - 1;
            return;
        }
        if(terminal[vis_term].current_location < terminal[vis_term].history_location){
            if(terminal[vis_term].current_location <0){
                terminal[vis_term].current_location = 0;
                return;
            }
            
            //delete the current screen text
            delete_current_text(0);

            //clear the buffer at the location to make sure there is no extra junk from any other commands
            clear_history_buffer_at_location(terminal[vis_term].history_location);

            //check the history variable to make sure its circulating
            history_count_check();

            //clear keyboard buffer to prepare for transfer 
            clear_keyboard_buffer();

            //clear terminal buffer to prepare for the new command
            //clear_terminal_buffer();

            //put the history in the keyboard to prepare for showing
            history_buffer_to_keyboard_buffer();

            //update the autofill buffer
            history_buffer_to_autofill_buffer();

            //show the history 
            history_to_screen(); 
            
            //load what we had in the keyboard buffer to the terminal buffer
            keyboard_buffer_to_vis_terminal_buffer(); 
            
            //update the arrow count to make sure it is valid
            update_current_location(scan_code);

        }
        up_press_count++;
    }

    //check if the up arrow key is pressed
    if(down_arrow){
        if(terminal[vis_term].current_location < terminal[vis_term].history_location){
            
            //delete the current screen text
            delete_current_text(0);

            //check the history variable to make sure its circulating
            history_count_check();

            //update the autofill buffer
            history_buffer_to_autofill_buffer();

            //clear the buffer at the location to make sure there is no extra junk from any other commands
            clear_history_buffer_at_location(terminal[vis_term].history_location);

            //clear keyboard buffer to prepare for transfer 
            clear_keyboard_buffer();

            //clear terminal buffer to prepare for the new command
            //clear_terminal_buffer();

            //put the history in the keyboard to prepare for showing
            history_buffer_to_keyboard_buffer();

            //show the history 
            history_to_screen(); 
            
            //load what we had in the keyboard buffer to the terminal buffer
            keyboard_buffer_to_vis_terminal_buffer();

            //update the arrow count to make sure it is valid
            update_current_location(scan_code); 
        }
        else{
            //delete the current screen text
            delete_current_text(0);

            //check the history variable to make sure its circulating
            history_count_check();

            //clear keyboard buffer to prepare for transfer 
            clear_keyboard_buffer();
        }
        down_press_count++;
    }
    //TODO: fix when up arrow pressed change the autofill buffer too!!!!!  FIX SPACE KEY PRESSED CASE!!!!
    if(scan_code == TAB_KEY && tab_pressed == ON){
        //compare each character in keyboard buffer with the list of executables
        int8_t fill_variable = check_autofill_match();

        if(fill_variable >= 0 && fill_variable <= 16){
            //delete what is on the screen
            if(terminal[vis_term].space_count != 0){
                delete_current_text(terminal[vis_term].buf_pos - strlen((void*)terminal[vis_term].autofill_buf));
            }
            else{
                delete_current_text(terminal[vis_term].buf_pos - strlen((void*)terminal[vis_term].keyboard_buf));
            }
    
            //clear the keyboard buffer
            //clear_keyboard_buffer();

            //fill the keyboard buffer with the correct executable
            unsigned char temp_key[KEY_BUF_SIZE];
            int temp_clear;
            for(temp_clear = 0; temp_clear<KEY_BUF_SIZE; temp_clear++){
                temp_key[temp_clear] = '\0';
            }
            strncpy((void*)temp_key, executable_list[fill_variable], strlen(executable_list[fill_variable]));

            //show what we have
            int j;
            j = 0;
            terminal[vis_term].autofill_location = 0;
            while(temp_key[j] != NULL){
                //buf_position--;
                add_to_keyboard_buffer((uint8_t)temp_key[j]);
                putc((uint8_t)temp_key[j]);
                j++;
            }

            //load what we have in the keyboard buffer to the terminal buffer
            keyboard_buffer_to_vis_terminal_buffer(); 
        }

        //if there is one match then fill each buffer with the respected characters
    }

    // prints the key to the screen using the scancode passed in
    if(shift == OFF && ((scan_code != UP_ARROW) && (scan_code != TAB_KEY) && (scan_code != DOWN_ARROW))){
        add_to_keyboard_buffer(keyboard_map[scan_code]);
        if(terminal[vis_term].buf_pos < KEY_BUF_SIZE){ //TODO: should this be visible terminal or cur terminal
            // check to make sure that you cannot back space to last terminal buffer
            if((keyboard_map[scan_code] != '\b') || (terminal[vis_term].can_backspace != 0)){
                putc(keyboard_map[scan_code]);
            }

        }
    }else if((scan_code != UP_ARROW) && (scan_code != DOWN_ARROW) && (scan_code != TAB_KEY)){
        add_to_keyboard_buffer(keyboard_map_shift[scan_code]);
        if(terminal[vis_term].buf_pos < KEY_BUF_SIZE){
            // check to make sure that you cannot back space to last terminal buffer
            if((keyboard_map_shift[scan_code] != '\b') || (terminal[vis_term].can_backspace != 0)){
                putc(keyboard_map_shift[scan_code]);
            }
        }
    }

    
    
}

/***
 * adds character to the keyboard buffer, capped at 128
 * if input exceeds buffer size it is ignored, backspace will 
 * delete last character in the buffer
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: adds character to keyboard_buf and changes buf_pos
 */
void add_to_keyboard_buffer(uint8_t c){
    // TODO: figure out how to handle the newline character and deleting to newline
    if((terminal[vis_term].buf_pos > 1) || (sizeof(terminal[vis_term].keyboard_buf) > 1)){
        terminal[vis_term].can_backspace = 1;
    }

    /*if(c=='\n'){
        terminal[vis_term].can_backspace = 0;
    }*/

    if (c==0x00){
        return;
    }


    if(c == '\b'){
        if(terminal[vis_term].buf_pos > 0){
            terminal[vis_term].buf_pos--;
            terminal[vis_term].autofill_location--;
        }
        else{
            terminal[vis_term].can_backspace = 0;
        }
        
        if(terminal[vis_term].keyboard_buf[terminal[vis_term].buf_pos-1] == ' '){
            terminal[vis_term].space_count--;
        }
        // buf_pos = ' ';
        terminal[vis_term].keyboard_buf[(terminal[vis_term].buf_pos)] = '\0';
        terminal[vis_term].autofill_buf[(terminal[vis_term].autofill_location)] = '\0';
        
    }else if (c == '\n'){
        //newline
        terminal[vis_term].keyboard_buf[terminal[vis_term].buf_pos] = c; 
        terminal[vis_term].can_backspace = 0;
        terminal[vis_term].enter_press = 1;
        terminal[vis_term].space_count = 0;
        terminal[vis_term].autofill_location = 0;
        //buf_pos = 0;
        keyboard_buffer_to_history_buffer();
        clear_autofill_buffer();
        keyboard_buffer_to_vis_terminal_buffer();
        clear_vis_keyboard_buffer();
        terminal[vis_term].current_location = terminal[vis_term].history_location - 1;
    }else{
        if(terminal[vis_term].buf_pos < KEY_BUF_SIZE){
            terminal[vis_term].keyboard_buf[terminal[vis_term].buf_pos] = c;
            terminal[vis_term].autofill_buf[terminal[vis_term].autofill_location] = c;
            terminal[vis_term].buf_pos++;
            terminal[vis_term].autofill_location++;
        }else{
            printf("Buffer Full");
            clear_vis_keyboard_buffer();
            clear_autofill_buffer();
            terminal[vis_term].autofill_location = -1;
            putc('\n');
            return;
        }
    }
}

/***
 * clears the keyboard buffer
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: resets keyboard buffer and buffer position
 */
void clear_keyboard_buffer(){
    int i;
    for(i = 0; i < KEY_BUF_SIZE; i++){
        terminal[cur_term].keyboard_buf[i] = 0x00;
    }
    terminal[cur_term].buf_pos = 0;
    terminal[cur_term].can_backspace = 0;
}


/***
 * clears the keyboard buffer for visible terminal
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: resets keyboard buffer and buffer position
 */
void clear_vis_keyboard_buffer(){
    int i;
    for(i = 0; i < KEY_BUF_SIZE; i++){
        terminal[vis_term].keyboard_buf[i] = 0x00;
    }
    terminal[vis_term].buf_pos = 0;
    terminal[vis_term].can_backspace = 0;
}

/***
 * clears the keyboard buffer
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: resets keyboard buffer and buffer position
 */
void clear_keyboard_buffer_at_location(int loc){
    int i;
    //i = loc;
    for(i = loc; i < KEY_BUF_SIZE; i++){
        terminal[vis_term].keyboard_buf[i] = 0x00;
    }
    terminal[vis_term].buf_pos = terminal[vis_term].current_location;
}

/***
 * copies current contents of the keyboard buffer to the terminal Buffer
 * INPUT: None 
 * OUTPUT: None 
 * EFFECTS: Copies keyboard buffer to terminal buffer
 */
void keyboard_buffer_to_terminal_buffer(){
    //memcpy(terminal_buffer, keyboard_buf,KEY_BUF_SIZE);
    int i;
    for(i = 0; i < KEY_BUF_SIZE;i++){
        terminal[cur_term].terminal_buf[i] = terminal[cur_term].keyboard_buf[i];
    }

}

/***
 * copies current contents of the keyboard buffer to the visual terminal Buffer
 * INPUT: None 
 * OUTPUT: None 
 * EFFECTS: Copies keyboard buffer to visual terminal buffer
 */
void keyboard_buffer_to_vis_terminal_buffer(){
    //memcpy(terminal_buffer, keyboard_buf,KEY_BUF_SIZE);
    int i;
    for(i = 0; i < KEY_BUF_SIZE;i++){
        terminal[vis_term].terminal_buf[i] = terminal[vis_term].keyboard_buf[i];
    }

}



/***
 * copies current contents of the keyboard buffer to the history Buffer
 * INPUT: None 
 * OUTPUT: None 
 * EFFECTS: Copies keyboard buffer to history buffer
 */
void keyboard_buffer_to_history_buffer(){
    int i;
    i = 0;
    if(terminal[vis_term].keyboard_buf[i] == '\n'){
        return;
    }
    for(i = 0; i < KEY_BUF_SIZE;i++){
        terminal[vis_term].history_buf[terminal[vis_term].history_location][i] = terminal[vis_term].keyboard_buf[i];
    }
    //history_buf[history_location][i] = 0x00;
    terminal[vis_term].history_location++;
}

/***
 * clears the history buffer
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: init clears the history buf
 */
void clear_history_buffer(){
    int i,j;
    for(i = 0; i < KEY_BUF_SIZE; i++){
        for(j = 0; j < HIST_BUF_SIZE; j++){
            terminal[vis_term].history_buf[j][i] = 0x00;
        }
    }
}

/***
 * puts the history buffer at the current location into the keyboard buffer 
 * to prepare for the add to keyboard buffer function
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: changes the keyboard buffer
 */
void history_buffer_to_keyboard_buffer(){
    //add the history to the kb buffer
    int i;
    for(i = 0; i < KEY_BUF_SIZE;i++){
        terminal[vis_term].keyboard_buf[i] = terminal[vis_term].history_buf[terminal[vis_term].current_location][i];
        if(terminal[vis_term].keyboard_buf[i] == '\n'){
            break;
        }
    }
}

void history_buffer_to_autofill_buffer(){
    //add the history to the autofill buffer
    int i;
    
    //clear the autofill buffer and reset location
    clear_autofill_buffer();
    terminal[vis_term].autofill_location = 0;
    
    for(i = 0; i < KEY_BUF_SIZE;i++){
        terminal[vis_term].autofill_buf[i] = terminal[vis_term].history_buf[terminal[vis_term].current_location][i];
        if(terminal[vis_term].keyboard_buf[i] == '\n'){
            break;
        }
    }
    //terminal[vis_term].autofill_location = strlen(terminal[vis_term].autofill_buf) + 1;
}



/***
 * Adds the keyboard to the keyboard buffer using the add to keyboard function
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: puts characters to the screen
 */
void history_to_screen(){
    int j;
    j = 0;
    while(terminal[vis_term].keyboard_buf[j] != NULL){
        //if there is no space add the key to everything we need to. if theres a space then reset the autofill and, still add to everything
        if(terminal[vis_term].keyboard_buf[j] != ' '){
                add_to_keyboard_buffer(terminal[vis_term].keyboard_buf[j]);
                putc(terminal[vis_term].keyboard_buf[j]);
            }
            else{
                terminal[vis_term].space_count++;
                clear_autofill_buffer();
                terminal[0].autofill_location = -1;
                add_to_keyboard_buffer(terminal[vis_term].keyboard_buf[j]);
                putc(terminal[vis_term].keyboard_buf[j]);
            }
        j++;
        //null check babyyyyyy
        if(terminal[vis_term].keyboard_buf[j+1] == NULL){
            break;
        }
    }
}

/***
 * Updates the current location based on if we use the up arrow or down arrow
 * INPUT: Scan code given by the keyboard interrupt (for this function it should only be up or down arrow)
 * OUTPUT: None
 * SIDE EFFECTS: increments or decrements the current location
 */
void update_current_location(int8_t scan_code){
    //check if the user clicked up or down
    if(scan_code == UP_ARROW){
        terminal[vis_term].current_location--;
        //if we decrement and its below 0, then set to 0.  this is so the user cant keep going up for no reason.  
        if(terminal[vis_term].current_location < 0){
            terminal[vis_term].current_location = 0;
        }
    }
    else if(scan_code == DOWN_ARROW){
        terminal[vis_term].current_location++;
        //if we increment and its above the history location, then set to 1 less than.  this is so the user cant keep going up for no reason. (security check, we already check this above but we do it again just in case)
        if(terminal[vis_term].current_location > terminal[vis_term].history_location){
            terminal[vis_term].current_location = terminal[vis_term].history_location - 1;
        }
    }
}

/***
 * Updates the history location, the variable needed to know where we should be in the array of histories to display.
 * INPUT: Scan code given by the keyboard interrupt (for this function it should only be up or down arrow)
 * OUTPUT: None
 * SIDE EFFECTS: increments or decrements the current location
 */
void history_count_check(){
    //if the history location is too big, set it back to 0 so we overwrite
    if(terminal[vis_term].history_location > HIST_BUF_SIZE){
        terminal[vis_term].history_location = 0;
    }
}

/***
 * Clears the history buffer at the given location.  This allows for there to never be junk inside if and when we circulate back
 * INPUT: integer location that is where we are pointing in the history array
 * OUTPUT: None
 * SIDE EFFECTS: clears the history buffer 
 */
void clear_history_buffer_at_location(int loc){
    int i;
    for(i=0; i<KEY_BUF_SIZE; i++){
        terminal[vis_term].history_buf[terminal[vis_term].history_location][i] = 0x00;
    }
}

/***
 * Delets the current text on the screen that the user has typed, this is so we can change commands
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: deletes from the screen and changes the keyboard buffer
 */
void delete_current_text(int loc){
    int i;
    i = loc;
    //if the keyboard buf at a current spot is empty then just return
    if(terminal[vis_term].keyboard_buf[i] == '\0'){
        return;
    }
    //iterate through the terminal at the given location and delete stuff
    while(terminal[vis_term].keyboard_buf[i] != '\n' || terminal[vis_term].keyboard_buf[i] != '\b'){
        if(terminal[vis_term].keyboard_buf[i] == '\n' || terminal[vis_term].keyboard_buf[i] == '\b' || terminal[vis_term].keyboard_buf[i] == '\0'){
            break;
        }
        terminal[vis_term].keyboard_buf[i] = '\0';
        putc('\b');
        terminal[vis_term].buf_pos--;
        i++;
    }
}

/***
 * clears the autofill buffer
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: puts null in for every character of 
 */
void clear_autofill_buffer(){
    int i;
    for(i = 0; i<KEY_BUF_SIZE; i++){
        terminal[vis_term].autofill_buf[i] = '\0';
    }
}

/***
 * checks if the user input is a match for any of the executables
 * INPUT: None
 * OUTPUT: Prints to the screen if there are multiple matches
 * RETURNS: the correct match if there is a singular match, or -1 on more than 1 or no valid matches
 * SIDE EFFECTS: prints to screen
 */
uint8_t check_autofill_match(){
    int p, q; //iteration variables to go through the lists
    int save_clear; //variable to clear the save buffer
    int maximum_match = 0; //maximum number of characters that match in a string
    int valid_check = 0; //how many strings match 
    int save_tracker = 0; //tracks what position of the save buffer we are in
    int main_save = 0; //used to put in what executable we are currently on
    int save_buf[32];  //variable to keep track of where most matched string are
    char temp[32]; //holds the current executable
    char save_keyboard[KEY_BUF_SIZE]; //saves whatever we have in the keyboard


    //clear the buffer we use to store the possible exe cmd
    for(save_clear = 0; save_clear<32; save_clear++){
        save_buf[save_clear] = 999;
    }

    //clear the buffer we use later to save the keyboard
    for(p = 0; p<KEY_BUF_SIZE; p++){
        save_keyboard[p] = '\0';
    }

    //iterate through each executable
    for(p = 0; p < sizeof(executable_list)/sizeof(executable_list[0]); p++){
        int k;
        //clear the temp array for each executable (32 is the size of temp)
        for(k = 0; k<32; k++){
            temp[k] = '\0';
        }
        //copy the executable to temp to check its match
        strncpy(temp, executable_list[p], strlen(executable_list[p]));
        autofill_match_count = 0;

        for(q = 0; q<sizeof(temp); q++){
            //if there is a match then update the match count, if the string character is null then break, if they are not equal set the match to 0 (because obv its not a match) and break the for loop
            if(temp[q] == terminal[vis_term].autofill_buf[q] && ((temp[q] != '\0' && temp[q] != '\b') && (terminal[vis_term].autofill_buf[q] != '\0' && terminal[vis_term].autofill_buf[q] != '\b'))){
                autofill_match_count++;
            }
            else if(terminal[vis_term].autofill_buf[q] == '\0' || terminal[vis_term].autofill_buf[q] == '\b')
            {
                break;
            }
            else if(temp[q] != terminal[vis_term].autofill_buf[q]){
                autofill_match_count = 0; 
                break;
            }
            else
            {
                autofill_match_count--;
            }
        }

        //if the match count is greater then update the match count 
        if(autofill_match_count > maximum_match){
            maximum_match = autofill_match_count;
            main_save = p;
            valid_check++;
            // if the match count is greater than what we have then add it to the saved array
            if(autofill_match_count >= strlen((void*)terminal[vis_term].autofill_buf)){
                save_buf[save_tracker] = p;
                save_tracker++;
            }
            
        }
        //if match with the max then reset the valid and add to the save buffer
        else if(autofill_match_count == maximum_match && maximum_match > 0){
            valid_check = 0;
            save_buf[save_tracker] = p;
            save_tracker++;
        }
    }

    //if the keyboard buf has nothing in it then return -1 
    if(strlen((void*)terminal[vis_term].keyboard_buf) == 0){
        return -1;
    }

    //return -1 if the first save buf is 999 (nothing important)
    if(save_buf[0] == 999){
        return -1;
    }
    
    //if there are matches but more than 1, then print out options
    if(valid_check == 0){
        printf("\nYour options are: ");
        int k;
        //print out the executables that the user might be trying to complete
        for(k = 0; k<17; k++){
            if(save_buf[k] != 999){
                printf((void*)executable_list[save_buf[k]]);
                printf(" ");
            }
        }

        //clear the autofill buffer to prepare for transfer
        clear_autofill_buffer();
        for(k = 0; k<strlen((void*)terminal[vis_term].keyboard_buf); k++){
            save_keyboard[k] = terminal[vis_term].keyboard_buf[k];
        }

        //clear the visible terminals keyboard to prepare to add back from the temp array we saved
        clear_vis_keyboard_buffer();
        terminal[vis_term].autofill_location = 0;
        printf("\n");

        //add every key back into the keyboard
        for(k = 0; k<KEY_BUF_SIZE; k++){
            if(save_keyboard[k] != ' '){
                add_to_keyboard_buffer(save_keyboard[k]);
                putc(save_keyboard[k]);
            }
            else{
                terminal[vis_term].space_count++;
                clear_autofill_buffer();
                terminal[vis_term].autofill_location = -1;
                add_to_keyboard_buffer(save_keyboard[k]);
                putc(save_keyboard[k]);
            }
            
            if(save_keyboard[k] == '\0'){
                break;
            }
        }
        //put the stuff back into the visible terminals keyboard
        keyboard_buffer_to_vis_terminal_buffer();
        return -1;
    }
    else{
        return main_save;  //mainsave is the fill match we got if we have a single valid match from the user
    }

}


