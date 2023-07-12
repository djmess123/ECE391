#include "types.h"
#include "lib.h"
#include "terminal.h"
#include "rtc.h"
#include "FileSys.h"


#ifndef _OP_TABLES_H
#define _OP_TABLES_H

/***
 * optable structure pointer
 */

typedef struct OP_TABLE{
    //uint32_t (*halt)(int8_t);
    //uint32_t (*execute)(const uint8_t);
    uint32_t (*read)(int32_t,void*,int32_t);
    uint32_t (*write)(int32_t, void*,int32_t);
    uint32_t (*open)(uint8_t*);
    uint32_t (*close)(int32_t);
} OP_TABLE_t;

/***
 * keith note: we might need to set read write open close to numbers 0-3.  
 * if we have one of these numbers then we set the pointer equal to whatever
 * function we need and store it somewhere?
 **/
// OP_TABLE_t terminal_op_table;
// terminal_op_table.read = read_terminal;
// terminal_op_table.write = write_terminal;
// terminal_op_table.open = open_terminal;
// terminal_op_table.close = close_terminal;









#endif /* _OP_TABLES_H  */