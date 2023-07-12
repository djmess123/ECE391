
#include "Paging.h"
#include "terminal.h"

#define PRESENT_FLAG 1
#define READWRITE_FLAG 2
#define USER_FLAG 4

/* void page_init( page_directory_entry_t * pageDir,  page_table_entry_t * pageTable);
 * Inputs: pageDir = pointer to page directory
           pageTable = pointer to the irst page table 0-4MB
 * Return Value: none
 * Function: initiates paging by editing control registers and intiating values in page dir and page table */
void page_init( page_directory_entry_t * pageDir,  page_table_entry_t * pageTable)
{
    int i;
    for(i=0;i<1024;i++){
        pageDir[i].tabAddress=0x00000002; //Set R/W bit to 1 so we can read and write to flag
    }
    for(i=0;i<1024;i++){
        pageTable[i].page=(i*FOURKB)|0x2;  //clear all pages
    }

    pageTable[VIDEO >> 12].page|= PRESENT_FLAG | USER_FLAG;  //VIDEO MEMORY: Clear lowest 12 bits, set R/W bit to 1 and present bit to 1
    pageTable[TERM_ONE_VIDEO >> 12].page|= PRESENT_FLAG | USER_FLAG;
    pageTable[TERM_TWO_VIDEO >> 12].page|= PRESENT_FLAG | USER_FLAG;
    pageTable[TERM_THREE_VIDEO >> 12].page|= PRESENT_FLAG| USER_FLAG;


    pageDir[0].tabAddress = ((uint32_t)pageTable & 0xFFFFF000) | 3; //Set first entry in the page directory to the table. set R/W, present bits to 1
    pageDir[1].tabAddress = FOURMB | 0x83;                          //set up kernal at 4mb, and set flags: PS = 1, SU = 0, RW = 1, P = 1


    loadPageDirectory(pageDir);
    enablePaging();
    return;
}

/***
 * sets the current program's page table to active
 * INPUT: address to virtualize
 * OUTPUT: none
 * EFFECTS: edits page directory
 */
void setup_paging(add){
    pageDir[32].tabAddress = add | 0x87; //TODO: comment out magic number
}

/***
 * sets the paging address of video memeory to address corresponding to terminal number passed in
 * INPUT: -1 resets video to default video memory, terminal sets vidmem to corresponding terminal
 * OUTPUT: None
 * EFFECTS: changes the video address of the video memory
 * (ZERO INDEXED so terminal numbers can be directly passed in)
 */
void set_vid_address(int terminal_num){
    pageDir[33].tabAddress =(uint32_t)pageTable|USER_FLAG|READWRITE_FLAG|PRESENT_FLAG;
    switch(terminal_num){
        case OUTPUT:
            pageTable[0].page=(VIDEO) | PRESENT_FLAG | READWRITE_FLAG | USER_FLAG;
            break;
        case 0:
            pageTable[0].page=(TERM_ONE_VIDEO) | PRESENT_FLAG | READWRITE_FLAG | USER_FLAG;
            break;
        case 1:
            pageTable[0].page=(TERM_TWO_VIDEO) | PRESENT_FLAG | READWRITE_FLAG | USER_FLAG;
            break;
        case 2:
            pageTable[0].page=(TERM_THREE_VIDEO) | PRESENT_FLAG | READWRITE_FLAG | USER_FLAG;
            break;
        default:
            printf("invalid terminal number passed in");
            return;
    }
    flush_tlb();
    
    return;
}

/***
 * sets the new virtual memory for vis term to output
 * INPUT: terminal to set as active
 * OUTPUT: none
 * EFFECTS: updates page table
 */
void set_vidmem(term){
    if(term == vis_term){
        set_vid_address(OUTPUT); //remaps video address to output 
    }else{
        set_vid_address(term);
    }
}

/***
 * saves output video memory to terminal's local storage
 * INPUT: terminal to save to
 * OUTPUT: none
 * EFFECTS: video memory, updates page table
 */
void save_vidmem(int terminal_num){
    if(cur_term==vis_term){
        pageTable[VIDEO>>12].page=(VIDEO) | PRESENT_FLAG | READWRITE_FLAG | USER_FLAG;
    }
    else
        pageTable[VIDEO>>12].page = ((VIDEO)+(terminal_num+1)*FOURKB) | PRESENT_FLAG | READWRITE_FLAG | USER_FLAG;

    flush_tlb();
}



