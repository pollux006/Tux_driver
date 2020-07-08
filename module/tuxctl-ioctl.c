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
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

// locks to protext shared resources
spinlock_t lock;
unsigned long lock_flag;
int ready;

// spinlock_t LED_lock;


char lednum[2][17]={{0xE7,0x06,0xCB,0x8F,0x2E,0xAD,0xED,0x86,0xEF,0xAF,0xEE,0x6D,0xE1,0x4F,0xE9,0xE8,0x00},
{0xF7,0x16,0xDB,0x9F,0x3E,0xBD,0xFD,0x96,0xFF,0xBF,0xFE,0x7D,0xF1,0x5F,0xF9,0xF8,0x10}
};  // the value used to set led from 0 to F and a empty(17)    the first 17 is no dot    second 17 vlaue is with dot on


static unsigned char pressed_button1; 
static unsigned char pressed_button2;  // save the current pressed button

static unsigned long led_value = 0; //initiate as 0
static int led_flag = 0;

//helper functions
/* get the current pressed button*/
int tux_init(struct tty_struct* tty);

/* get the current pressed button*/
int tux_buttons(struct tty_struct* tty , unsigned long arg);

/* set led */
int set_led(struct tty_struct* tty, unsigned long arg);





/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet(struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	char op;
	// unsigned op;
	// int suc;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    // printk("packet : %x %x %x\n", a, b, c);

	switch(a){
		case MTCP_ACK:
			ready = 1; // set device ready
			if(led_flag){
				set_led(tty,led_value);		// init and reset led value
				led_flag=0;
			}
			
			
			return;
		case MTCP_BIOC_EVENT:
			spin_lock_irqsave(&lock,lock_flag);
			pressed_button1 = ~b; // get the package and ~ because of realtive low
			pressed_button2 = ~c;
			spin_unlock_irqrestore(&lock,lock_flag);
			return;

		case MTCP_RESET:
			
			
			pressed_button1 = 0; // clear presssed button
			pressed_button2 = 0; // clear pressed button
			
			led_flag = 1;
			// BIOC_ON to enable interrupt to input button value
			// MTCP_LED_USR puts LED setting into usermode
			op = MTCP_LED_USR; // send two command
			tuxctl_ldisc_put(tty,&op,1);

			op = MTCP_BIOC_ON;
			tuxctl_ldisc_put(tty,&op,1);
			
			
			// spin_unlock_irqrestore(&lock,lock_flag);
			return;
		default:
			return;


	}

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
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
	int suc;
	// char op;
	
    switch (cmd) {
	case TUX_INIT:
		suc = tux_init(tty); // call init helper
		return suc;

	case TUX_BUTTONS:
		if (arg == 0){
			return -EINVAL; // check argument
		}
		suc = tux_buttons(tty, arg);
		return suc;

	case TUX_SET_LED:
		suc = set_led(tty,arg);

	default:
	    return -EINVAL;
    }

	return -EINVAL;
}






/* tux_buttons(struct tty_struct* tty,unsigned long arg)
 *
 * description : handle the command from pc and return the package 
 *				
 * INPUT:   struct tty_struct* tty
 * 			unsigned long arg --> the pointer to user space
 * 
 * OUTPUT:  int -> sucessful or not
 * 
 * SIDE_EFFECT:  return the button pressed in the format of   right,left,down,up,c,b,a
 * 
 */

int tux_buttons(struct tty_struct* tty,unsigned long arg){
	int i;
	unsigned char back= 0;
	
	spin_lock_irqsave(&lock,lock_flag);

	back = pressed_button1 & 0x0F; // lower bits are just copied
	back += (pressed_button2 & 0x08)<<4; // 0x08 is 00001000  which is right and left shift by 4 to highest
	back += (pressed_button2 & 0x02)<<5; // 0x02 is 00000010 which is left and shift 5 to the second highest
    back += (pressed_button2 & 0x04)<<3; // 0x04 is 00000100 whcih is down and shift by 3 to the third highest
	back += (pressed_button2 & 0x01)<<4; // 0x01 is 00000001 whichis start and shift by four to the fourth highest
	


	i = copy_to_user((int*)arg, &back, 1);   // copy to user

	spin_unlock_irqrestore(&lock,lock_flag);

	if (i == 0){
		return 0;		// check sucess
	}
	return 1;
}



/* tux_init(struct tty_struct* tty)
 *
 * description : init the tux controller 
 *				
 * INPUT:   struct tty_struct* tty
 * 
 * OUTPUT:  int -> sucessful or not
 * 
 * SIDE_EFFECT: Enable Button interrupt-on-change. 
 * 				Put the LED display into user-mode. 
 *  			Set device ready to 1
 * 
 */

int tux_init(struct tty_struct* tty){
	
		char op;
		char suc;
		ready = 0;
		spin_lock_init(&lock);
		
		pressed_button1 = 0; // clear presssed button
		pressed_button2 = 0; // clear pressed button
		// BIOC_ON to enable interrupt to input button value
		// MTCP_LED_USR puts LED setting into usermode
		
		op = MTCP_BIOC_ON;
		suc = tuxctl_ldisc_put(tty,&op,1);
		op = MTCP_LED_USR; // send two command
		suc = tuxctl_ldisc_put(tty,&op,1);
		ready = 1;
		return suc;
}




/* tux_led(struct tty_struct* tty, unsigned long arg)
 *
 * description : set the led on the tux controller
 *				
 * INPUT:   struct tty_struct* tty
 * 			usigned long arg --> argument send to tux
 * 
 * OUTPUT:  int -> sucessful or not
 * 
 * SIDE_EFFECT: 
 * 				Set the led according to the argument send
 * 
 * 
 */
int set_led(struct tty_struct* tty, unsigned long arg){

	char op;
	int suc;
	int i;
	// first get the number
	char led;
	char num[4];
	char dot[4]; // 4 number and 4 dot

	if (ready == 0){
		return -EINVAL;
	} // if device not ready just return
	ready = 0;

			// set not ready
	
	led_value = arg;		// save arg

	led = ((arg)>>(16))&0x0F;	// shift by 16 and & with 1111 gets the lower 4 bits of third byte

	for(i=0;i<4;i++){	// go through four led   (i is led offset)

		num[i] = ((arg)>>(4*i))&0x0F;		// number to just shift by 4 bits * (led offset)
		dot[i] = ((arg)>>(i+24))&1;			// get the lower 4 bits of highest byte (shift 24) and find the dp value with offset i
	}

	



	spin_lock_irqsave(&lock,lock_flag);
	op = MTCP_LED_SET;				
	suc = tuxctl_ldisc_put(tty,&op,1);		// first send command to start setting
	op = 0x0F;  							// always redraw four led
	suc = tuxctl_ldisc_put(tty,&op,1);		
	
	for(i=0;i<4;i++){		// go through 4 leds
		if((led>>i)&1){			//if the mask said we need to draw that led

			op = lednum[(int)dot[i]][(int)num[i]];		// get the value from pre-defined array
			tuxctl_ldisc_put(tty,&op,1);		//send
		
		}
		else{
			op = lednum[(int)dot[i]][16];   // i designed index 16 as the empty at the lednum[] array
			tuxctl_ldisc_put(tty,&op,1);
		
		}
	}
	spin_unlock_irqrestore(&lock,lock_flag);
	return 0;
}



