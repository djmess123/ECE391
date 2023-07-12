

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define TABLE_SIZE 1024
#define VIDEO       0xB8000
#define TERM_ONE_VIDEO 0xB9000
#define TERM_TWO_VIDEO 0xBA000
#define TERM_THREE_VIDEO 0xBB000
#define FIRST_LOC   0x800000
#define SECOND_LOC  0xC00000
#define VIRTUAL_LOC 0x08000000
#define FOURMB  0x00400000
#define FOURKB 0x1000
#define PROGIND     128/4
#define OUTPUT -1

typedef uint32_t page_t;

typedef struct {
    page_t page;
} page_table_entry_t;

typedef struct {
    page_t tabAddress;
} page_directory_entry_t;

page_directory_entry_t pageDir[1024] __attribute__((aligned (4096))); //1024 = number of pages tables (or 4MB pages) in a page directory. alligned to 4kB
page_table_entry_t pageTable[1024] __attribute__((aligned (4096))); //1024 = number of pages in page table. alligned to 4kB
page_table_entry_t pageTable_USR[1024] __attribute__((aligned (4096))); //1024 = number of pages in page table. alligned to 4kB
//bookmark


extern void loadPageDirectory(page_directory_entry_t * pageDir);
extern void enablePaging(void);
extern void page_init( page_directory_entry_t * pageDir,  page_table_entry_t * pageTable);
extern void setup_paging(int32_t add);
extern void flush_tlb();

void set_vid_address(int terminal_num);
extern void save_vidmem(int);
//function sets the video memory to the apropriate terminal
void set_vidmem(int term);

#endif
