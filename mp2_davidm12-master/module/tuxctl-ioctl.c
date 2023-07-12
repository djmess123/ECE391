/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	//printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

//indexed bytes representing the hex display LED command arrangement
char segHex[16] = { 0xE7,
                    0x06,
                    0xCB,
                    0x8F,
                    0x2E,
                    0xAD,
                    0xED,
                    0x86,
                    0xEF,
                    0xAE,
                    0xEE,
                    0x6D,
                    0xE1,
                    0x4F,
                    0xE9,
                    0xE8    };

typedef enum buttonState {
    RELEASED = 0,
    PRESSED,
    HELD
} buttonState_t ;

typedef struct inputRecord {
    buttonState_t up;
    buttonState_t right;
    buttonState_t down;
    buttonState_t left;
    buttonState_t a;
    buttonState_t b;
    buttonState_t c;
    buttonState_t start;
    buttonState_t reset;

} inputRecord_t;

inputRecord_t ir = {0};


typedef struct LEDRecord {
    int waitingForACK;
    unsigned int LEDStates[4];
    int waitingForReset;
} LEDRecord_t;

LEDRecord_t lr = {0};

spinlock_t tuxLock = SPIN_LOCK_UNLOCKED;



/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
    unsigned char buf[9] = {0};
    a = packet[0]; /* Avoid //printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    //printk("response packet : %x %x %x    aka: ", a, b, c);

    //verify valid packet
    if (((a & 0xC8) != 0x40) || ((b & 0x80) != 0x80) || ((c & 0x80) != 0x80))
    {
        //printk("BAD PACKET\n");
        return;
    }

    //decide what kind of repsonse
    //char opcode = (a & 0x07) + ((a & 0x30) >> 1);
    
    switch(a)
    {
        case MTCP_ERROR:
            //printk("ERROR");
            break;
        case MTCP_ACK:
            //printk("ACK");
            spin_lock(&tuxLock);
            lr.waitingForACK = 0;   //reset LED wait
            spin_unlock(&tuxLock);
            break;
        case MTCP_POLL_OK:
            break;

        case MTCP_RESET:
            //printk("RESET");
            tuxLock = SPIN_LOCK_UNLOCKED;
            buf[0] = MTCP_LED_USR;      //allow LEDs to be set
            buf[1] = MTCP_BIOC_ON;      //allow button interrupts
            buf[3] = MTCP_LED_SET;      //Put LEDs back on last save
            buf[4] = 0xF;
            //spin_lock(&tuxLock);
            buf[5] = lr.LEDStates[0];
            buf[6] = lr.LEDStates[1];
            buf[7] = lr.LEDStates[2];
            buf[8] = lr.LEDStates[3];
            //send command packet
            tuxctl_ldisc_put(tty, buf, 9);
            lr.waitingForReset = 0;
            lr.waitingForACK = 0;
            //spin_unlock(&tuxLock);
            break;
        case MTCP_OFF_EVENT:
            //printk("TURNING OFF");
            //(save/restore the LED state!)
            spin_lock(&tuxLock);
            lr.waitingForReset = 1;
            spin_unlock(&tuxLock);
            break;
        case MTCP_BIOC_EVENT:
            //printk("BIOC");
            spin_lock(&tuxLock);
            ir.start =  (0 != (packet[1] & (1 << 0)));  //unpack button statuses
            ir.a =      (0 != (packet[1] & (1 << 1)));
            ir.b =      (0 != (packet[1] & (1 << 2)));
            ir.c =      (0 != (packet[1] & (1 << 3)));
            ir.up =     (0 != (packet[2] & (1 << 0)));
            ir.left =   (0 != (packet[2] & (1 << 1)));
            ir.down =   (0 != (packet[2] & (1 << 2)));
            ir.right =  (0 != (packet[2] & (1 << 3)));
            spin_unlock(&tuxLock);
            
            break;
    



        case MTCP_LEDS_POLL0:
        case __LEDS_POLL01:
        case __LEDS_POLL02:
        case __LEDS_POLL012:
        break;
        case MTCP_LEDS_POLL1:
        case __LEDS_POLL11:
        case __LEDS_POLL12:	
        case __LEDS_POLL112:
        break;
        
        
        

    }
    //printk("\n    +Finished handler\n");
    //decifer data into proper variables

    //kick off specific handlers?


    return;

}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	              unsigned cmd, unsigned long arg)
{
    int num;
    char mask;
    char decimal;
    int active;
    int i;
    char buf[6];
    char buildBuf[6];
    ////printk("ioctl!!!\n");
    switch (cmd) 
    {
	case TUX_INIT:                                  //init Tux and driver
        (void) arg;
        //printk("INIT REQ\n");
        //reset???
        //init driver variables
        //tuxLock = SPIN_LOCK_UNLOCKED;
        lr.waitingForReset = 0;
        lr.LEDStates[0] = 0;
        lr.LEDStates[1] = 0;
        lr.LEDStates[2] = 0;
        lr.LEDStates[3] = 0;
        
        //init TUX?
        
        //generate packet
        buf[0] = MTCP_LED_USR; //allow LEDs to be set
        buf[1] = MTCP_BIOC_ON; //allow button interrupts
        //send command packet
        tuxctl_ldisc_put(tty, buf, 2);
        lr.waitingForACK = 1;
        break;

	case TUX_BUTTONS:                               //grab latest button values
        //printk("BUTTON REQ\n");
        if (arg == 0)
            return -EINVAL;

        buf[0] = 0;
        spin_lock(&tuxLock);
        buf[0] = ir.start          +                //format buttons
                (ir.a <<        1) +
                (ir.b <<        2) +
                (ir.c <<        3) +
                (ir.up <<       4) +
                (ir.down <<     5) +
                (ir.left <<     6) +
                (ir.right <<    7);

        copy_to_user((unsigned long *)arg, buf, 1);  //copy 1 byte
        spin_unlock(&tuxLock);
        break;

	case TUX_SET_LED:                               //set LED values
        //printk("%d", lr.waitingForACK);
        if (lr.waitingForACK)
            return -EINVAL;
        //printk("SET_LED REQ\n");
        num =       0x0000FFFF & arg;           //stores number across 4 blocks using lower 2 byte mask
        mask =     (0x000F0000 & arg) >> 16;    //stores what LED blocks to use using 3rd byte lower 4 bit mask
        decimal =  (0x0F000000 & arg) >> 24;    //stores what decimal points using 4th byte lower 4 mask
        i = 0;
        //calculate the individual LED command bytes
        if(mask & 0x1)
            {buildBuf[i] = segHex[num & 0x000F]          + (0x10 * (0 != (decimal & 0x1)));  ++i;}
        else
            {buildBuf[i] = 0; ++i;}

        if(mask & 0x2)
            {buildBuf[i] = segHex[(num & 0x00F0) >> 4]   + (0x10 * (0 != (decimal & 0x2)));  ++i;}
        else
            {buildBuf[i] = 0; ++i;}

        if(mask & 0x4)
            {buildBuf[i] = segHex[(num & 0x0F00) >> 8]   + (0x10 * (0 != (decimal & 0x4)));  ++i;}
        else
            {buildBuf[i] = 0; ++i;}

        if(mask & 0x8)
            {buildBuf[i] = segHex[(num & 0xF000) >> 12]  + (0x10 * (0 != (decimal & 0x8)));  ++i;}
        else
            {buildBuf[i] = 0; ++i;}



        //generate packet
        buf[0] = MTCP_LED_SET;
        //check delta
        active = 0;
        buf[1] = 0;
        spin_lock(&tuxLock);
        for(i = 0; i < 4; ++i)
        {
            if (buildBuf[i] != lr.LEDStates[i])
            {
                lr.LEDStates[i] = buildBuf[i];       //update latest state
                buf[2 + active] = buildBuf[i];       //add to packet
                ++active;
                buf[1] += (1 << i);
            }
        }
        //send command packet
        tuxctl_ldisc_put(tty, buf, 2 + active);
        lr.waitingForACK = 1;
        spin_lock(&tuxLock);
        //printk("LED packet : %x %x %x %x %x %x  len: %d\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], active);

        break;
	default:
	    return -EINVAL;
    }
    //printk("    +Finished request\n");

    return 0;
}


/*
i = 2;
    if(mask & 0x01){
        buf[i] = segHex[num & 0x000F];
        ++i;
    }
    if(mask & 0x02){
        buf[i] = segHex[(num & 0x00F0) >> 4];
        ++i;
    }
    if(mask & 0x04){
        buf[i] = segHex[(num & 0x0F00) >> 8];
        ++i;
    }
    if(mask & 0x08){
        buf[i] = segHex[(num & 0xF000) >> 12];
        ++i;
    }

*/


