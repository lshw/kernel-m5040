/******************************************************************************
 * Copyright (C) 2004 Silicon Motion                                          *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc.,                                                          *
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
 ******************************************************************************/

/******************************************************************************
 * Driver support for the on-chip VoyagerGX dual-channel serial port,         *
 * running in asynchronous mode.  Also, support for doing a serial console    *
 * on one of those ports.                                                     *
 *                                                                            *
 * The non-console part of this code is based heavily on the sb1250_duart.c   *
 * driver also in this directory.  See tty_driver.h for a description of some *
 * of the driver functions, though it (like most of the inline code           *
 * documentation :) is a bit out of date.                                     *
 *                                                                            *
 * Note:                                                                      *
 *   CONFIG_SMIVGX_UART_OUTPUT_BUF_SIZE is defined in Config.in               *
 ******************************************************************************/

#include <linux/types.h>
#include <linux/serial.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/console.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>
#include <linux/major.h>
#include <linux/termios.h>
#include <linux/spinlock.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/tty_flip.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include "smivgx.h"

//#define request_irq myrequest_irq 
/*
 * Version
 */
#define DUART_VERSION		1.0
#define DUART_DATE		04/02/2004


#define spin_lock_irqsave    
#define spin_unlock_irqrestore 

/*
 * Debug output control
 */
#undef DUART_SPEW
#define DUART_ERROR_MESSAGES
#define DUART_DEBUG_PREFIX "SMI_VGX_UART "


/*
 * Global definitions
 */
#define JIFFIES_PER_SECOND	100
#define DUART_SEND_WAIT		(JIFFIES_PER_SECOND * 3)

#define OUTPUT_BUF_SIZE	512
#define TX_INTEN		1

/*
 * Default transmission flags
 */
#define INPUT_CLOCK_RATE	8000000		/* 8MHz */
#define DEFAULT_BAUD_RATE	115200
#define DEFAULT_CFLAGS		(CS8 | B115200)


/*
 * Minimum maco
 */
#define MIN(a, b) (((a)<(b))?(a):(b))


/*
 * Driver object structures
 */
static struct tty_driver *smivgx_duart_driver;


/*
 * Reference count variable
 */
static int ref_count;


/*
 * Still not sure what the termios structures set up here are for,
 * but we have to supply pointers to them to register the tty driver
 */
static struct tty_struct *duart_table[2];
static struct termios    *duart_termios[2];
static struct termios    *duart_termios_locked[2];


/*
 * PCI definitions
 */
#ifndef PCI_VENDOR_ID_SILICONMOTION
#define PCI_VENDOR_ID_SILICONMOTION			0x126F
#endif

#ifndef PCI_DEVICE_ID_SILICONMOTION_SM501
#define PCI_DEVICE_ID_SILICONMOTION_SM501	0x0501
#endif

#define SM501_MAGIC \
	((PCI_VENDOR_ID_SILICONMOTION << 16) | \
	 PCI_DEVICE_ID_SILICONMOTION_SM501)

#define SM501_EXTENT		(2 * 1024 * 1024)

#define SM501_MODULE_NAME	"smivgx_serial"
#define PFX					SM501_MODULE_NAME ": "


/*
 * Register access definitions
 */
#define UART_REG_SPACING \
	(UART_1_RECEIVE_BUFFER - UART_0_RECEIVE_BUFFER)

/* Write to transmit, Read to receive */
#define UART_DATA_REG(line) \
	(UART_0_RECEIVE_BUFFER + (line) * UART_REG_SPACING)

/* Read Only */
#define UART_LINE_STATUS_REG(line) \
	(UART_0_LINE_STATUS + (line) * UART_REG_SPACING)

/* Read Only */
#define UART_INT_STATUS_REG(line) \
	(UART_0_STATUS + (line) * UART_REG_SPACING)

/* Read/Write */
#define UART_INTMASK_REG(line) \
	(UART_0_INTERRUPT + (line) * UART_REG_SPACING)

/* Write Only */
#define UART_FIFO_CONTROL_REG(line) \
	(UART_0_FIFO + (line) * UART_REG_SPACING)

/* Read/Write */
#define UART_LINE_CONTROL_REG(line) \
	(UART_0_LINE_CTRL + (line) * UART_REG_SPACING)

/* Write Only (?) */
#define UART_DIVIDER_LOW_REG(line) \
	(UART_0_DIVISOR_LATCH_LSB + (line) * UART_REG_SPACING)

/* Write Only (?) */
#define UART_DIVIDER_HIGH_REG(line) \
	(UART_0_DIVISOR_LATCH_MSB + (line) * UART_REG_SPACING)


/*
 * Trigger values
 */
#define FIFO_LENGTH		64

#define TRIGGER_DEFAULT	UART_0_FIFO_RX_TRIGGER_DEFAULT
#define TRIGGER_QUARTER	UART_0_FIFO_RX_TRIGGER_QUARTER
#define TRIGGER_HALF	UART_0_FIFO_RX_TRIGGER_HALF
#define TRIGGER_MINUS_2	UART_0_FIFO_RX_TRIGGER_MINUS_2

static unsigned int trigger_treshold[] =
{
	1, 16, 32, 62
};


/*
 * This lock protects both the open flags for all the uart states as
 * well as the reference count for the module
 */
static spinlock_t open_lock = SPIN_LOCK_UNLOCKED;


/*
 * Bit fields of flags in the flags field below
 */
#define SD_WRITE_WAKE       0x000000001


/*
 * Bit fields of flags in the flags field below
 */
typedef struct
{
	struct tty_struct   *tty;
	unsigned char       outp_buf[CONFIG_SMIVGX_UART_OUTPUT_BUF_SIZE];
	unsigned int        outp_head;
	unsigned int        outp_tail;
	unsigned int        outp_count;
	spinlock_t          outp_lock;
	unsigned int        outp_stopped;
	unsigned int        outp_transmitting;
	unsigned int        open;
	unsigned long       flags;
	unsigned int        last_cflags;
	unsigned char       rx_fifo_trigger;
	unsigned char       tx_fifo_trigger;
	unsigned char       fifo_control_value;
}
uart_state_t, *puart_state_t;

uart_state_t uart_states[2] = { [0 ... 1] = {
tty:                0,
					outp_head:          0,
					outp_tail:          0,
					outp_lock:          SPIN_LOCK_UNLOCKED,
					outp_count:         0,
					outp_transmitting:  0,
					open:               0,
					flags:              0,
					last_cflags:        0,
					rx_fifo_trigger:    0,
					tx_fifo_trigger:    0,
					fifo_control_value: 1,
}};


/*
 * System-level info
 */
typedef struct
{
	/* Magic. */
	unsigned int        magic;

	/* The corresponding pci_dev structure. */
	struct pci_dev*     dev;
	unsigned int        device;
	unsigned int        revision;

	/* Hardware resources. */
	unsigned int        phys_io;
	unsigned char*      io;
	unsigned int        irq;
}
smivgx_t, *psmivgx_t;

psmivgx_t drv_data;
int pci_session;


/******************************************************************************
 * MODULE PARAMETERS                                                          *
 ******************************************************************************/

static int io = -1;
static int irq = -1;

//module_param(io, int ,-1);
MODULE_PARM_DESC(io, "sets base address for hostbus interface");
//module_param(irq, int , -1);
MODULE_PARM_DESC(irq, "sets IRQ for hostbus interface");

void __init smivgx_serial_console_init(void);
/******************************************************************************
 * UTILITY FUNCTIONS FOR OUTPUT BUFFER                                        *
 ******************************************************************************/

unsigned char isempty(puart_state_t obj)
{
	return (obj->outp_count == 0);
}

unsigned char isfull(puart_state_t obj)
{
	return (obj->outp_count >= CONFIG_SMIVGX_UART_OUTPUT_BUF_SIZE);
}

unsigned int getvacant(puart_state_t obj)
{
	return (CONFIG_SMIVGX_UART_OUTPUT_BUF_SIZE - obj->outp_count);
}

unsigned char getchar(puart_state_t obj)
{
	unsigned char ch = 0;

	if (obj->outp_count)
	{
		ch = obj->outp_buf[obj->outp_head];
		obj->outp_head = (obj->outp_head + 1) &
			(CONFIG_SMIVGX_UART_OUTPUT_BUF_SIZE - 1);
		obj->outp_count--;
	}

	return ch;
}

static inline int copy_buf(char *dest, const char *src, int size, int from_user)
{
	if (from_user)
	{
		copy_from_user(dest, src, size);
	}
	else
	{
		memcpy(dest, src, size);
	}

	return size;
}

unsigned int storedata(puart_state_t obj,
		int from_user, const unsigned char* buf, int count)
{
	int chars_written = 0;


	/* Copy the data */
	if (isfull(obj))
	{
		printk(DUART_DEBUG_PREFIX "[storedata]: the buffer is full!\n");
	}
	else
	{
		if (obj->outp_tail < obj->outp_head)
		{
			/* Straightforward case; copy from tail to head */
			chars_written = copy_buf(obj->outp_buf + obj->outp_tail, buf,
					MIN(count, obj->outp_head - obj->outp_tail), from_user);
		}
		else
		{
			/* Copy from tail to end of buffer, wrap around and then
			   copy to head */
			chars_written = copy_buf(obj->outp_buf + obj->outp_tail, buf,
					MIN(CONFIG_SMIVGX_UART_OUTPUT_BUF_SIZE - obj->outp_tail, count),
					from_user);

			if (chars_written < count)
			{
				chars_written += copy_buf(obj->outp_buf, buf + chars_written,
						MIN(obj->outp_head, count - chars_written), from_user);
			}
		}

		/* Update the pointers */
		obj->outp_tail = (obj->outp_tail + chars_written) &
			(CONFIG_SMIVGX_UART_OUTPUT_BUF_SIZE - 1);
		obj->outp_count += chars_written;

#ifdef DUART_SPEW
		printk(DUART_DEBUG_PREFIX "[storedata]: tail = %d\n", obj->outp_tail);
		printk(DUART_DEBUG_PREFIX "[storedata]: head = %d\n", obj->outp_head);
		printk(DUART_DEBUG_PREFIX "[storedata]: written = %d\n", chars_written);
#endif
	}

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[storedata]: --\n");
#endif

	return chars_written;
}

/******************************************************************************
 * REGISTER ACCESS                                                            *
 ******************************************************************************/

inline unsigned int PEEK(psmivgx_t s, unsigned int address)
{
	register unsigned int value = readl(s->io + address);
	rmb();
	return value;
}

inline void POKE(psmivgx_t s, unsigned int address, unsigned int value)
{
	writel(value, s->io + address);
	wmb();
}

inline unsigned char PEEKBYTE(psmivgx_t s, unsigned int address)
{
	register unsigned char value = readb(s->io + address);
	rmb();
	return value;
}

inline void POKEBYTE(psmivgx_t s, unsigned int address, unsigned char value)
{
	writeb(value, s->io + address);
	wmb();
}




/******************************************************************************
 * INLINE FUNCTIONS LOCAL TO THIS MODULE                                      *
 ******************************************************************************/

/*
 * Mask out the passed interrupt lines at the duart level.  This should be
 * called while holding the associated outp_lock
 */
static inline void duart_disable_ints(psmivgx_t s,
		unsigned int line, unsigned char mask)
{
	unsigned char value;

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_disable_ints]: ++(mask = 0x%02X)\n", mask);
#endif

	value = PEEKBYTE(s, UART_INTMASK_REG(line));
	value &= ~mask;
	POKEBYTE(s, UART_INTMASK_REG(line), value);

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_disable_ints]: --\n");
#endif
}


/*
 * Unmask the passed interrupt lines at the duart level
 */
static inline void duart_enable_ints(psmivgx_t s,
		unsigned int line, unsigned char mask)
{
	unsigned char value;

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_enable_ints]: ++(mask = 0x%02X)\n", mask);
#endif

	value = PEEKBYTE(s, UART_INTMASK_REG(line));
	value |= mask;
	POKEBYTE(s, UART_INTMASK_REG(line), value);

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_enable_ints]: --\n");
#endif
}


/*
 * Read the status register
 */
static inline unsigned char get_line_status_reg(psmivgx_t s, unsigned int line)
{
	return PEEKBYTE(s, UART_LINE_STATUS_REG(line));
}


/*
 * Read the interrupt status register
 */
static inline unsigned char get_int_status_reg(psmivgx_t s, unsigned int line)
{
	return PEEKBYTE(s, UART_INT_STATUS_REG(line));
}


/*
 * Derive which uart a call is for from the passed tty line
 */
static inline unsigned int get_line(struct tty_struct *tty)
{
	unsigned int line = tty->index;
	return 0;
}




/******************************************************************************
 * INTERRUPT HANDLING                                                         *
 ******************************************************************************/

/*
 * Called from smivgx_interrupt_entry with a specific
 * UART interrupt to process
 */
static void smivgx_uart_interrupt(psmivgx_t s, unsigned int line)
{
	unsigned int int_status;
	puart_state_t obj = &uart_states[line];

	//printk("int int int\n");
	/* Read the interrupt status register */
	//	int_status = get_int_status_reg(s, line);


	/* Receive the data first */
	//	if (TEST_FIELD(int_status, UART_0_STATUS_INTERRUPT, RX_DATA_RECEIVED) )
	{

		/* Read the interrupt status register */


		unsigned int i;
		unsigned char status;
		unsigned char ch;
		unsigned int flag;
		//printk("11111111111111\n");


		for (i = 0; ; i++)
		{
			/* Make sure we have something in the buffer */
			//printk("1.55555555555555\n");
			status = get_line_status_reg(s, line);
			//printk("222222222222222222\n");
			if (!TEST_FIELD(status, UART_0_LINE_STATUS_DATA, READY))
				break;

			/* Read the data */
			ch = PEEKBYTE(s, UART_DATA_REG(line));

			/* Reset the error flag */
			flag = TTY_NORMAL;


			//printk("____hex :%x  char:  %c\n",ch,ch);
			/* Insert the character */
			tty_insert_flip_char(obj->tty, ch, flag);
		}


		//	tty_flip_buffer_push(obj->tty);
		tty_schedule_flip(obj->tty);	


	}

}


/*
 * Generic interrupt handler for both channels.  dev_id is a pointer
 * to the proper uart_states structure, so from that we can derive
 * which port interrupted
 */
static int smivgx_interrupt_entry(int irq, void *dev_id, struct pt_regs *regs)
{
	uart_state_t *us = (uart_state_t *)dev_id;
	struct tty_struct *tty = us->tty;
	unsigned int value;
	int line = 0;
	psmivgx_t s = (psmivgx_t)dev_id;
	unsigned int int_status, line_status;

	int_status = PEEKBYTE(s, UART_INT_STATUS_REG(line));
	if (TEST_FIELD(int_status, UART_0_STATUS_INTERRUPT, RX_DATA_RECEIVED) ||
			TEST_FIELD(int_status, UART_0_STATUS_INTERRUPT, RX_CHAR_TIMEOUT)){
		int counter = 2048;
		unsigned int ch;
		line_status = get_line_status_reg(s,line);
		if (TEST_FIELD(line_status, UART_0_LINE_STATUS_OVERRUN, ERROR))
			tty_insert_flip_char(tty, 0, TTY_OVERRUN);
		if (TEST_FIELD(line_status, UART_0_LINE_STATUS_PARITY, ERROR)) {
#ifdef DUART_SPEW
			printk("Parity error!\n");
#endif
		}
		while (counter > 0) {
			line_status = get_line_status_reg(s,line);
			if(TEST_FIELD(line_status, UART_0_LINE_STATUS_DATA, NOT_READY))
				break;
			ch = PEEKBYTE(s,UART_DATA_REG(line));
			tty_insert_flip_char(tty, ch, 0);
			//__________udelay(1);
			counter--;
		}
		tty_flip_buffer_push(tty);
	}

	if (TEST_FIELD(int_status, UART_0_STATUS_INTERRUPT, TX_EMPTY)) {
		smivgx_uart_interrupt(s,0);
	}
#ifdef DUART_SPEW
	printk("[smivgx_interrupt_entry:] --\n");
#endif
	return IRQ_HANDLED;
	//printk("<0>smivgx_interrupt_entry\n");
	/* Determine which UART raised the interrupt */
	//value = PEEK(s, INTERRUPT_STATUS);
	//POKE(s,INTERRUPT_STATUS,value);

	//	if (TEST_FIELD(value, INTERRUPT_STATUS_UART0, ACTIVE))
	//		smivgx_uart_interrupt(s, 0);

	//	if (TEST_FIELD(value, INTERRUPT_STATUS_UART1, ACTIVE))
	//		smivgx_uart_interrupt(s, 1);
	//	return 1;
}


/******************************************************************************
 * ACTUAL DRIVER FUNCTIONS                                                    *
 ******************************************************************************/

/*
 * Return the number of characters we can
 * accomodate in a write at this instant
 */
static int duart_write_room(struct tty_struct *tty)
{
	return 1024;
}


/*
 * The function enables the actual data transmission
 */
static void start_transmisson(unsigned int line)
{
	puart_state_t obj = &uart_states[line];

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[start_transmisson]: ++(uart%d)\n", line);
#endif

	do
	{
		/* Make sure that the transmission
		   has not been forced to stop */
		if (obj->outp_stopped)
		{
#ifdef DUART_SPEW
			printk(DUART_DEBUG_PREFIX "[start_transmisson]: cannot start - duart_stop has been called\n");
#endif
			break;
		}

		/* If outp_count is not zero, then the
		   transmission is in progress already */
		if (obj->outp_transmitting)
		{
#ifdef DUART_SPEW
			printk(DUART_DEBUG_PREFIX "[start_transmisson]: already started\n");
#endif
			break;
		}

		/* Set the transmit fifo trigger level */
		//		set_tx_trigger(drv_data, line, TRIGGER_DEFAULT);

		/* Enable transmit interrupt */
		duart_enable_ints(drv_data, line, GET_MASK(UART_0_INTERRUPT_TX_EMPTY));
		obj->outp_transmitting = 1;
	}
	while (0);

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[start_transmisson]: --\n");
#endif
}


/*
 * The function waits until the transfer is complete or
 * until the specified timeout is reached
 */
static int wait_until_sent(unsigned int line, int timeout)
{
	int result = 0;
	uart_state_t *obj = &uart_states[line];
	unsigned long target_time = jiffies + timeout;

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[wait_until_sent]: ++(timeout = %d, uart%d)\n",
			timeout, line);
#endif

	while (1)
	{
		/* Stop if timed out */
		if (jiffies > target_time)
		{
#ifdef DUART_SPEW
			printk(DUART_DEBUG_PREFIX "[wait_until_sent]: timed out\n");
#endif
			result = 1;
			break;
		}

		/* Stop if everything is sent */
		if (isempty(obj))
		{
#ifdef DUART_SPEW
			printk(DUART_DEBUG_PREFIX "[wait_until_sent]: the buffer is empty\n");
#endif
			break;
		}

		/* Wait a bit */
		schedule_timeout(1);
	}

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[wait_until_sent]: --\n");
#endif
	return result;
}


/*
 * Buffer up to count characters from buf to be written.
 * If we don't have other characters buffered, enable the
 * tx interrupt to start sending
 */
static int duart_write(struct tty_struct* tty,const unsigned char* buf, int count)
{

	uart_state_t *us; 
	int c, t, total = 0; 
	unsigned long flags;

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_write]: ++(%i chars by %i (%s))\n", count, current->pid, current->comm);
#endif

	if (!tty) return 0;

	us = tty->driver_data;
	if (!us) return 0;

	spin_lock_irqsave(&us->outp_lock, flags);

	for (;;) {
		c = count;

		t = OUTPUT_BUF_SIZE - us->outp_tail;
		if (t < c) c = t; 

		t = OUTPUT_BUF_SIZE - 1 - us->outp_count;
		if (t < c) c = t; 

		if (c <= 0) break;

		memcpy(us->outp_buf + us->outp_tail, buf, c);

		us->outp_count += c;
		us->outp_tail = (us->outp_tail + c) & (OUTPUT_BUF_SIZE - 1);
		buf += c;
		count -= c;
		total += c;
	}

	spin_unlock_irqrestore(&us->outp_lock, flags);

	if (us->outp_count && !tty->stopped && !tty->hw_stopped && !(us->flags & TX_INTEN)) {
		us->flags |= TX_INTEN;
		if (0 == tty->index) {
			duart_enable_ints(drv_data,0, GET_MASK(UART_0_INTERRUPT_TX_EMPTY));
		} else {
			duart_enable_ints(drv_data,1, GET_MASK(UART_1_INTERRUPT_TX_EMPTY));
		}
		us->outp_transmitting = 1;
	}

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_write]: --(wrote %d character(s))\n", total);
#endif

	return total;
#if 0
	int i;
	int line;
	if(!tty) return 0;
	line = get_line(tty);


	/* Send the data */
	for (i = 0; i < count; i++)
	{	
		//		printk("tx hex: %x char: %c\n",buf[i] ,buf[i]);

		while((PEEK(drv_data,UART_LINE_STATUS_REG(line))&0x20) == 0);
		POKEBYTE(drv_data, UART_DATA_REG(line), buf[i]);

	}

	return count;
#endif
}




/*
 * Return the number of characters in the output
 * buffer that have yet to be written
 */
static int duart_chars_in_buffer(struct tty_struct *tty)
{
	int retval;
	unsigned long flags;
	unsigned int line = get_line(tty);
	uart_state_t* obj = &uart_states[line];

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_chars_in_buffer]: ++(uart%d)\n", line);
#endif

	/* Enter the critical section */
	spin_lock_irqsave(&obj->outp_lock, flags);

	retval = obj->outp_count;
	if (retval == CONFIG_SMIVGX_UART_OUTPUT_BUF_SIZE)
	{
		obj->flags |= SD_WRITE_WAKE;
	}

	/* Leave the critical section */
	spin_unlock_irqrestore(&obj->outp_lock, flags);

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_chars_in_buffer]: --(%i)\n", retval);
#endif

	return retval;
}


/*
 * Kill everything we haven't yet shoved into the FIFO.
 * Turn off the transmit interrupt since we've nothing more to transmit
 */
static void duart_flush_buffer(struct tty_struct *tty)
{

}


/*
 * Handle notification of a termios change.
 */
static void duart_set_termios(struct tty_struct *tty, struct termios *old)
{

}


/*
 * Stop pushing stuff into the fifo, now.
 * Do the mask under the outp_lock to avoid races
 * involving turning the interrupt line on/off
 */
static void duart_start(struct tty_struct *tty)
{
	unsigned long flags;
	unsigned int line = get_line(tty);
	uart_state_t *obj = &uart_states[line];

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_start]: ++(uart%d)\n", line);
#endif

	/* Enter the critical section */
	spin_lock_irqsave(&obj->outp_lock, flags);

	/* Reset the stop flag */
	obj->outp_stopped = 0;

	/* Enable transmission interrupt */
	start_transmisson(line);

	/* Leave the critical section */
	spin_unlock_irqrestore(&obj->outp_lock, flags);

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_start]: --\n");
#endif
}


/*
 * Stop pushing stuff into the fifo, now.
 * Do the mask under the outp_lock to avoid races
 * involving turning the interrupt line on/off
 */
static void duart_stop(struct tty_struct *tty)
{
	unsigned long flags;
	unsigned int line = get_line(tty);
	uart_state_t *obj = &uart_states[line];

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_stop]: ++(uart%d)\n", line);
#endif

	/* Enter the critical section */
	spin_lock_irqsave(&obj->outp_lock, flags);

	/* Disable transmission interrupt */
	duart_disable_ints(drv_data, line, GET_MASK(UART_0_INTERRUPT_TX_EMPTY));

	/* Set the stop flag */
	obj->outp_stopped = 1;
	obj->outp_transmitting = 0;

	/* Leave the critical section */
	spin_unlock_irqrestore(&obj->outp_lock, flags);

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_stop]: --\n");
#endif
}


/*
 * IOCTL function dispatcher
 */
static int duart_ioctl(struct tty_struct *tty, struct file * file,
		unsigned int cmd, unsigned long arg)
{
	int result = -ENOIOCTLCMD;
	return result;
}

static int duart_compat_ioctl(struct tty_struct *tty, struct file * file,
		unsigned int cmd, unsigned long arg)
{
	int result = -ENOIOCTLCMD;
	printk("--------------compat_ioctl\n");
	return result;
}

/*
 * Not sure on the semantics of this; are we supposed to wait until the stuff
 * already in the hardware FIFO drains, or are we supposed to wait until
 * we've drained the output buffer, too?  I'm assuming the former, 'cause thats
 * what the other drivers seem to assume
 */
static void duart_wait_until_sent(struct tty_struct *tty, int timeout)
{
	unsigned int line = get_line(tty);

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_wait_until_sent]: ++(timeout = %d, uart%d)\n",
			timeout, line);
#endif

	wait_until_sent(line, timeout);

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_wait_until_sent]: --\n");
#endif
}

/*
 * Initializes the FIFO
 */
void init_fifo(unsigned int line)
{
	unsigned int value = uart_states[line].fifo_control_value;
#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[init_fifo]: fifo_control_value is 0x%x\n", value);
#endif
	/*
	 * FIXME:
	 * _Need one elegant way
	 */
	if (line == 0) { 
		value = SET_FIELD(value, UART_0_FIFO_TX_CLEAR, CLEAR);
		value = SET_FIELD(value, UART_0_FIFO_RX_CLEAR, CLEAR);
	} else {
		value = SET_FIELD(value, UART_1_FIFO_TX_CLEAR, CLEAR);
		value = SET_FIELD(value, UART_1_FIFO_RX_CLEAR, CLEAR);
	}
	value = SET_FIELD(value, UART_0_FIFO_ENABLE, ENABLE);
	POKEBYTE(drv_data,UART_FIFO_CONTROL_REG(line), value);
#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[init_fifo]: value is 0x%x\n", value);
#endif
}

/*
 * Open a tty line.  Note that this can be called multiple times,
 * so ->open can be >1.  Only set up the tty struct if this is
 * a "new" open, e.g. ->open was zero
 */
static int duart_open(struct tty_struct *tty, struct file *filp)
{
	uart_state_t* obj;
	unsigned long flags;
	//unsigned int line = get_line(tty);
	unsigned int line = 0;

	//printk("--------------duart open --------0\n");
	/* Make sure we support the requested line */
	if (line > 1) 
		return -ENODEV;
	//printk("--------------duart open --------1\n");
	init_fifo(0);
	/* Increase the reference counter */

	tty->driver_data = NULL;
	obj = &uart_states[line];
	tty->driver_data = obj;

	/* Enter the critical section */
	spin_lock_irqsave(&open_lock, flags);
	if (!obj->open) {
		obj->tty = tty;
		obj->tty->termios->c_cflag = obj->last_cflags;
	}
	obj->open++;
	obj->flags &= ~0x1;
	//uart_states[0].tty=tty;

#ifdef	CONFIG_SMIVGX_EXT_CLOCK
	/*set GPIO Clock as uart's input	--by boyod */	
	value = PEEK(drv_data,MISCELLANEOUS_TIMING);
	value = SET_FIELD(value, MISCELLANEOUS_TIMING_UART0_PLL, ENABLE);
	POKE(drv_data,MISCELLANEOUS_TIMING, value);
#endif

	//	duart_enable_ints(drv_data, line, 0x3);
	//	duart_enable_ints(drv_data, line, 0x1);
	duart_enable_ints(drv_data, line, GET_MASK(UART_0_INTERRUPT_RX_BUFFER));

	{ 
		unsigned int value;
		value = PEEK(drv_data, INTERRUPT_MASK);
		value = SET_FIELD(value, INTERRUPT_MASK_UART0, ENABLE);
		POKE(drv_data, INTERRUPT_MASK, value);
	}

	spin_unlock_irqrestore(&open_lock, flags);
//	printk("--------------duart open --------2\n");
	return 0;
}

/*
 * Close a reference count out.  If reference count hits zero, null the
 * tty, kill the interrupts.  The tty_io driver is responsible for making
 * sure we've cleared out our internal buffers before calling close()
 */
static void duart_close(struct tty_struct *tty, struct file *filp)
{
	unsigned long flags;
	unsigned int line = get_line(tty);
	uart_state_t *obj = &uart_states[line];

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_close]: ++(%i (%s), uart%d)\n",
			current->pid, current->comm, line);
#endif

	/* Enter the critical section */
	spin_lock_irqsave(&open_lock, flags);

	obj->open--;
	ref_count--;

	if (!obj->open)
	{
		unsigned int value;

		duart_disable_ints(drv_data, line, 0xFF);

		switch (line)
		{
			case 0:
				value = PEEK(drv_data, INTERRUPT_MASK);
				value = SET_FIELD(value, INTERRUPT_MASK_UART0, DISABLE);
				POKE(drv_data, INTERRUPT_MASK, value);

				value = PEEK(drv_data, GPIO_CONTROL_HIGH);
				value = SET_FIELD(value, GPIO_CONTROL_HIGH_37, GPIO);
				value = SET_FIELD(value, GPIO_CONTROL_HIGH_38, GPIO);
				value = SET_FIELD(value, GPIO_CONTROL_HIGH_39, GPIO);
				value = SET_FIELD(value, GPIO_CONTROL_HIGH_40, GPIO);
				POKE(drv_data, GPIO_CONTROL_HIGH, value);

				value = PEEK(drv_data, CURRENT_GATE);
				value = SET_FIELD(value, CURRENT_GATE_UART0, DISABLE);
				//			set_power(drv_data, value, PEEK(drv_data, CURRENT_CLOCK));
				break;

			case 1:
				value = PEEK(drv_data, INTERRUPT_MASK);
				value = SET_FIELD(value, INTERRUPT_MASK_UART1, DISABLE);
				POKE(drv_data, INTERRUPT_MASK, value);

				value = PEEK(drv_data, MISCELLANEOUS_CONTROL);
				value = SET_FIELD(value, MISCELLANEOUS_CONTROL_UART1_SELECT, SSP);
				POKE(drv_data, MISCELLANEOUS_CONTROL, value);

				value = PEEK(drv_data, GPIO_CONTROL_HIGH);
				value = SET_FIELD(value, GPIO_CONTROL_HIGH_41, GPIO);
				value = SET_FIELD(value, GPIO_CONTROL_HIGH_42, GPIO);
				value = SET_FIELD(value, GPIO_CONTROL_HIGH_43, GPIO);
				value = SET_FIELD(value, GPIO_CONTROL_HIGH_44, GPIO);
				POKE(drv_data, GPIO_CONTROL_HIGH, value);

				value = PEEK(drv_data, CURRENT_GATE);
				value = SET_FIELD(value, CURRENT_GATE_UART1, DISABLE);
				//set_power(drv_data, value, PEEK(drv_data, CURRENT_CLOCK));

				break;
		}
	}

	/* Leave the critical section */
	spin_unlock_irqrestore(&open_lock, flags);

	/* Decrease the reference counter */

#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[duart_close]: --\n");
#endif
}

/*
 * VoyagerGX deinitialization
 */
static void __devinit cleanup(psmivgx_t s)
{
	unsigned int ret;
	unsigned int value;


	if (s == NULL)
		return;


	ret = tty_unregister_driver(smivgx_duart_driver);
	if (ret)
		printk(DUART_DEBUG_PREFIX "[cleanup]: unable to unregister voyagergx duart serial driver (%d)\n", ret);

	if (s->irq)
	{
		/* Remove interrupt service handler. */
		synchronize_irq();
		free_irq(s->irq, s);
	}

	if (s->io != NULL)
	{
		/* Disable interrupts */
		value = PEEK(s, INTERRUPT_MASK);
		value = SET_FIELD(value, INTERRUPT_MASK_UART0, DISABLE);
		value = SET_FIELD(value, INTERRUPT_MASK_UART1, DISABLE);
		POKE(s, INTERRUPT_MASK, value);

		/* Disable UART0 and UART1. */
		value = PEEK(s, CURRENT_GATE);
		value = SET_FIELD(value, CURRENT_GATE_UART0, DISABLE);
		value = SET_FIELD(value, CURRENT_GATE_UART1, DISABLE);
		//set_power(s, value, PEEK(s, CURRENT_CLOCK));

		/* Unmap I/O space. */
		iounmap(s->io);
		release_mem_region(s->phys_io, SM501_EXTENT);
	}

	/* Free state structure. */
	kfree(s);

}

static inline void set_rx_trigger(unsigned int line, unsigned int trigger)
{
	puart_state_t	obj = &uart_states[line];
#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[set_rx_trigger]: ++\n");
	printk(DUART_DEBUG_PREFIX "[set_rx_trigger]: trigger is 0x%x\n", trigger);
	printk(DUART_DEBUG_PREFIX "[set_rx_trigger]: obj->fifo_control_value is 0x%x\n", obj->fifo_control_value);
#endif
	if (line == 0) {
		obj->rx_fifo_trigger = trigger & RAW_MASK(UART_0_FIFO_RX_TRIGGER);
		obj->fifo_control_value = SET_FIELDV(obj->fifo_control_value,UART_0_FIFO_RX_TRIGGER, obj->rx_fifo_trigger);
	} else {
		obj->rx_fifo_trigger = trigger & RAW_MASK(UART_1_FIFO_RX_TRIGGER);
		obj->fifo_control_value = SET_FIELDV(obj->fifo_control_value,UART_1_FIFO_RX_TRIGGER, obj->rx_fifo_trigger);
	}
	POKE(drv_data,UART_FIFO_CONTROL_REG(line), obj->fifo_control_value);
#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[set_rx_trigger]: --\n");
#endif
}

static inline void set_tx_trigger(unsigned int line, unsigned int trigger)
{
	puart_state_t	obj = &uart_states[line];
#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[set_tx_trigger]: ++\n");
	printk(DUART_DEBUG_PREFIX "[set_tx_trigger]: trigger is 0x%x\n", trigger);
	printk(DUART_DEBUG_PREFIX "[set_tx_trigger]: obj->fifo_control_value is 0x%x\n", obj->fifo_control_value);
#endif
	if (line == 0) {
		obj->tx_fifo_trigger = trigger & RAW_MASK(UART_0_FIFO_TX_TRIGGER);
		obj->fifo_control_value = SET_FIELDV(obj->fifo_control_value, UART_0_FIFO_TX_TRIGGER, obj->tx_fifo_trigger);
	} else {
		obj->tx_fifo_trigger = trigger & RAW_MASK(UART_1_FIFO_TX_TRIGGER);
		obj->fifo_control_value = SET_FIELDV(obj->fifo_control_value, UART_1_FIFO_TX_TRIGGER, obj->tx_fifo_trigger);
	}
#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[set_tx_trigger]: init fifo reg 0x%02x\n", obj->fifo_control_value);
#endif
	POKE(drv_data,UART_FIFO_CONTROL_REG(line), obj->fifo_control_value);
#ifdef DUART_SPEW
	printk(DUART_DEBUG_PREFIX "[set_tx_trigger]: --\n");
#endif
}

static void duart_hw_init(void)
{
	unsigned int	value;

	value = PEEK(drv_data, GPIO_CONTROL_HIGH);
	value = SET_FIELD(value, GPIO_CONTROL_HIGH_37, UART0_TX);
	value = SET_FIELD(value, GPIO_CONTROL_HIGH_38, UART0_RX);
	POKE(drv_data, GPIO_CONTROL_HIGH, value);

	duart_disable_ints(drv_data,0, 0xff);

	set_rx_trigger(0, TRIGGER_HALF);
	/* Set the transmit fifo trigger level */
	set_tx_trigger(0, TRIGGER_DEFAULT);
}

/*
 * VoyagerGX initialization
 */
static int smivgx_init(struct platform_device *pdev)
{
	int res = -1;
	psmivgx_t s = NULL;
	unsigned int value;
	struct pci_dev *pcidev=container_of(pdev->dev.parent,struct pci_dev,dev);
	//prom_printf("smivgx_init enter\n");

	/* Init the pointers */
	drv_data = NULL;
	pci_session = -1;

	/* Allocate the state structure. */
	s = kmalloc(sizeof(smivgx_t), GFP_KERNEL);
	if (s == NULL)
	{
		printk(DUART_DEBUG_PREFIX "[smivgx_init]: out of memory\n");
		return -ENOMEM;
	}
	memset(s, 0, sizeof(smivgx_t));

	do
	{

		/* Enable PCI device. */
		//		res = pci_enable_device(pcidev);

		drv_data = s;
		pci_session = 0;
		//pci_set_drvdata(pcidev, drv_data);
		smivgx_duart_driver = alloc_tty_driver(2);
		/* Register the driver */
		smivgx_duart_driver->driver_name      = "serial";

		smivgx_duart_driver->name             = "ttyS";
		smivgx_duart_driver->major            = TTY_MAJOR;
		smivgx_duart_driver->minor_start      = 64;
	  //smivgx_duart_driver.num               = 2;
		smivgx_duart_driver->type             = TTY_DRIVER_TYPE_SERIAL;
		smivgx_duart_driver->subtype          = SERIAL_TYPE_NORMAL;
		smivgx_duart_driver->init_termios     = tty_std_termios;
		smivgx_duart_driver->flags            = TTY_DRIVER_REAL_RAW;
		smivgx_duart_driver->refcount         = &ref_count;
	  //smivgx_duart_driver->table            = duart_table;
		smivgx_duart_driver->termios          = duart_termios;
		smivgx_duart_driver->termios_locked   = duart_termios_locked;

		smivgx_duart_driver->ops->open             = duart_open;
		smivgx_duart_driver->ops->close            = duart_close;
		smivgx_duart_driver->ops->write            = duart_write;
		smivgx_duart_driver->ops->write_room       = duart_write_room;
		smivgx_duart_driver->ops->chars_in_buffer  = duart_chars_in_buffer;
		smivgx_duart_driver->ops->flush_buffer     = duart_flush_buffer;
		smivgx_duart_driver->ops->ioctl            = duart_ioctl;
		//smivgx_duart_driver->ops->compat_ioctl     = duart_compat_ioctl;
		smivgx_duart_driver->ops->set_termios      = duart_set_termios;
		smivgx_duart_driver->ops->stop             = duart_stop;
		smivgx_duart_driver->ops->start            = duart_start;
		smivgx_duart_driver->ops->wait_until_sent  = duart_wait_until_sent;

		/* Interrupts are now active, our ISR can be called. */
		res = tty_register_driver(smivgx_duart_driver);
		if (res)
			printk(DUART_DEBUG_PREFIX "[smivgx_init]: " \
					"couldn't register voyagergx duart serial driver\n");

		/* Initialize state structure. */
		s->magic   = SM501_MAGIC;
		s->dev     = pcidev;
		s->phys_io = (pcidev == NULL) ? io : pci_resource_start(pcidev, 1);
		s->irq     = (pcidev == NULL) ? irq : pcidev->irq;

		s->io = ioremap_nocache(s->phys_io, SM501_EXTENT);
		//prom_printf("s->io=%llx\n",s->io);
		if (s->io == NULL)
		{
			printk(DUART_DEBUG_PREFIX "[smivgx_init]: " \
					"unable to map memory %08x-%08x\n",
					s->phys_io, s->phys_io + SM501_EXTENT - 1);
			res = -EBUSY;
			break;
		}
		smivgx_serial_console_init();

		/* Read device ID from I/O space. */
		value = PEEK(s, DEVICE_ID);
		s->device = GET_FIELD(value, DEVICE_ID_DEVICE);
		s->revision = GET_FIELD(value, DEVICE_ID_REVISION);

		printk(DUART_DEBUG_PREFIX "[smivgx_init]: " \
				"found chip, device=%04x revision=%02x io=%p irq=%u\n",
				s->device, s->revision, s->io, s->irq);

		printk(DUART_DEBUG_PREFIX "[smivgx_init]: transmit buffer length = %d\n",
				CONFIG_SMIVGX_UART_OUTPUT_BUF_SIZE);

		duart_hw_init();

		/* Set interrupt service handler. */
		res = request_irq(36, smivgx_interrupt_entry, SA_SHIRQ, SM501_MODULE_NAME, drv_data);

		if (res != 0)
			panic(DUART_DEBUG_PREFIX "[smivgx_init]: unable to request irq %u\n", 36);

		return 0;
	}
	while (0);

	/* Uninitialize anything we have done so far. */
	cleanup(s);

	return res;
}

/*
 * VoyagerGX removal
 */
static void __devinit smivgx_remove(struct platform_device *pdev)
{
	struct pci_dev* pcidev=container_of(pdev->dev.parent,struct pci_dev,dev);


	/* cleanup state structure pointer. */
	cleanup((pci_session == 0)? drv_data : pci_get_drvdata(pcidev));

	if (pci_session == 1)
		pci_set_drvdata(pcidev, NULL);

	pci_session = -1;
	drv_data = NULL;

}

/*
 * PCI initialization structures
 */
static struct pci_device_id id_table[] __devinitdata =
{
	{ PCI_VENDOR_ID_SILICONMOTION, PCI_DEVICE_ID_SILICONMOTION_SM501, PCI_ANY_ID, PCI_ANY_ID, 0, 0 },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, id_table);

static struct platform_driver smivgx_duart_pci_driver = {
	.probe		= smivgx_init,
	.remove		= smivgx_remove,
	.driver		= {
		.name	= "sm501-uart",
		.owner	= THIS_MODULE,
	},
};

/*
 * Set up the driver and register it, register the 2 UART interrupts.
 * This is called from tty_init, or as a part of the module init
 */
static int __init smivgx_duart_init(void)
{
	int res = -ENODEV;
	res = platform_driver_register(&smivgx_duart_pci_driver);

	return res;
}

/*
 * Unload the driver.  Unregister stuff, get ready to go away
 */
static void __exit smivgx_duart_fini(void)
{

	platform_driver_unregister(&smivgx_duart_pci_driver);

}

/******************************************************************************
 * MODULE ENTRY POINTS AND DESCRIPTION                                        *
 ******************************************************************************/

module_init(smivgx_duart_init);
module_exit(smivgx_duart_fini);

#ifdef CONFIG_SMIVGX_SERIAL_CONSOLE

static void ser_console_write(struct console *cons, const char *str,
		unsigned int count)
{
	unsigned int i;
	unsigned long flags;
	unsigned int line = 0;

	/* Enter the critical section */

	for (i = 0; i < count; i++)
	{
		if (str[i] == '\n')
		{
			/* Expand LF -> CRLF */
			while((PEEK(drv_data,UART_LINE_STATUS_REG(line))&0x20) == 0);
			POKEBYTE(drv_data, UART_DATA_REG(line), '\r');
		}

		while((PEEK(drv_data,UART_LINE_STATUS_REG(line))&0x20) == 0);
		POKEBYTE(drv_data, UART_DATA_REG(line), str[i]);
	}

}

static struct tty_driver * ser_console_device(struct console *c,int *index)
{
	*index = c->index;
	return smivgx_duart_driver;
}

static int ser_console_setup(struct console *cons, char *str)
{
	return 0;
}

static struct console smivgx_ser_cons =
{
name:		"ttyS",
			write:		ser_console_write,
			device:		ser_console_device,
			setup:		ser_console_setup,
			flags:		CON_PRINTBUFFER,
			index:		-1,
};

void __init smivgx_serial_console_init(void)
{
	register_console(&smivgx_ser_cons);
}

#endif /* CONFIG_SERIAL_CONSOLE */


