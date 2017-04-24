/******************************************************************************\
 *
 * $Workfile:   sm501.c  $
 *
 * Silicon Motion VoyagerGX SM501 audio driver.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (c) 2004, Himalaya Software
 * Copyright (c) 2008, Siliconmotion corp.
 *
  *
 *    Rev 1.7   Jun 03 2008 14:22:00   boyod
 *	Add recorde function.
 *
 *    Rev 1.6   Apr 31 2008 15:00:00   boyod
 *	upgrate to 2.6.21, Base SM501 core interface module.
 *
 *    Rev 1.5   Apr 13 2004 18:35:40   alexei
 * Created a wrapper for read_codec and write_codec to be used when called
 * from the outside, so that the functions could be called from the inside
 * without having to grab the semaphore, which could be already grabbed by
 * the caller function, which would lead to the driver waiting infinitely
 * and causing the whole system to hang. The lock situation has been noticed
 * during the init, when wait_codec is called by the ac97 engine and
 * wait_codec in turn calls read_codec.
 *
 *    Rev 1.4   Mar 17 2004 11:59:44   frido
 * Added DEBUG flag.
 * 
 *    Rev 1.3   Mar 17 2004 11:56:38   frido
 * Added version number 1.0.1.
 * Changed some KERROR into KWARNING.
 * Added VBR flag.
 * 
 *    Rev 1.2   Mar 16 2004 23:41:48   frido
 * First working driver on 44.1kHz.
 * 
 *    Rev 1.1   Mar 04 2004 23:12:02   frido
 * Tired of bug hunting outside my driver.
 * 
 *    Rev 1.0   Feb 22 2004 12:09:40   frido
 * Initial Revision
 *
\******************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
//#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/sound.h>
#include <linux/slab.h>
#include <linux/soundcard.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/bitops.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/smp_lock.h>
#include <linux/ac97_codec.h>
#include <linux/platform_device.h>
//#include <linux/wrapper.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>

#include <linux/sm501.h>
#include <linux/sm501_regs.h>
#include "sm501fw.h"


/* -------------------------------------------------------------------------- */

#undef OSS_DOCUMENTED_MIXER_SEMANTICS

#define KERROR(msg, args...) printk(KERN_ERR PFX msg, ##args)
#define KWARNING(msg, args...) printk(KERN_WARNING PFX msg, ##args)
//#define KDEBUG(msg, args...)                       printk(KERN_INFO PFX msg, ##args)
#ifdef CONFIG_SOUND_SM501_DEBUG
#   define KDEBUG(msg, args...) printk(KERN_DEBUG PFX msg, ##args)
#else
#   define KDEBUG(msg, args...) do {} while(0)
#endif

/* -------------------------------------------------------------------------- */

#ifndef PCI_VENDOR_ID_SILICONMOTION
#define PCI_VENDOR_ID_SILICONMOTION         0x126F
#endif

#ifndef PCI_DEVICE_ID_SILICONMOTION_SM501
#define PCI_DEVICE_ID_SILICONMOTION_SM501   0x0501
#endif

#define SM501_MAGIC     ((PCI_VENDOR_ID_SILICONMOTION << 16) | \
                         PCI_DEVICE_ID_SILICONMOTION_SM501)

#define SM501_EXTENT    (2 * 1024 * 1024)

#define SM501_MODULE_NAME   "sm501"
#define PFX                 SM501_MODULE_NAME ": "

/* -------------------------------------------------------------------------- */

typedef struct
{
    /* Buffer stuff. */
    unsigned hw_buffer;
    unsigned hw_buffer_size;
    unsigned char* sw_buffer;
    unsigned sw_buffer_size;
    unsigned hw_ptr, sw_ptr;
    unsigned total_bytes;
    int count;
    unsigned error;
    int threshold;
    /* Kernel suff. */
    wait_queue_head_t wait;
    /* Format suff. */
    int bits;
    int channels;
    int rate;
    unsigned int enabled : 1;
    unsigned int ready : 1;
    unsigned int endcleared : 1;
    unsigned int running : 1;
    unsigned int vbr : 1;
}
sm501BufferRec, *sm501BufferPtr;

typedef struct
{
    /* Magic. */
    unsigned int magic;

    /* The corresponding pci_dev structure. */
    struct device * dev;
    unsigned int device;
    unsigned int revision;

    /* Soundcore stuff. */
    int dev_audio;
    struct ac97_codec* codec;
    sm501BufferRec playing;
    sm501BufferRec recording;

    /* Hardware resources. */
    unsigned int phys_io;
    unsigned char* io;
    unsigned int irq;

#ifdef CONFIG_SOUND_SM501_DEBUG
    /* debug /proc entry */
    struct proc_dir_entry* ps;
    struct proc_dir_entry* ac97ps;
    unsigned int* samples;
    unsigned int sample_index;
    unsigned int sample_size;
    unsigned char* data;
    unsigned int data_index;
    unsigned int data_size;
    struct proc_dir_entry* sample_ps;
#endif

    spinlock_t lock;
    struct semaphore open_sem;
    mode_t open_mode;
    wait_queue_head_t open_wait;
    struct semaphore sem;
}
sm501Rec, *sm501Ptr;

static sm501Ptr host_data;
static ulong CodecPowerState;

/* -------------------------------------------------------------------------- */

static const char invalid_magic[] = KERN_CRIT PFX "invalid magic value\n";
static int i_flag = 15;
	
#define VALIDATE_STATE(s) \
    do \
    { \
		if ((s) == NULL) \
		{ \
		    printk(invalid_magic); \
		    return(-ENXIO); \
		} \
    } \
    while (0)

	/* -------------------------------------------------------------------------- */

	#define LOWORD(x) ((unsigned short) (x))
	#define HIWORD(x) ((unsigned short) ((x) >> 16))
	#define LOBYTE(x) ((unsigned char) (x))
	#define HIBYTE(x) ((unsigned char) ((x) >> 8))

	#define MAKEWORD(l,h)  (((unsigned short) (unsigned char) (l)) | \
			       (((unsigned short) (unsigned char) (h)) << 8))
	#define MAKEDWORD(l,h) (((unsigned int) (unsigned short) (l)) | \
			       (((unsigned int) (unsigned short) (h)) << 16))

	#define SRAM(x)      (U8051_SRAM + offsetof(FIRMWARE_SRAM, x))
	#define SRAM_SIZE(x) (sizeof(((FIRMWARE_SRAM*) 0)->x))


static int write_command(sm501Ptr s, unsigned char command, unsigned int data0,
				 unsigned int data1)
{
	    int i;
	    /* Reset status. */
	    SmWrite8(SRAM(status_byte), 0);

	    /* Fill command and data bytes. */
	    KDEBUG("write_command: %u %08x %08x\n", command, data0, data1);
	    SmWrite8(SRAM(command_byte), command);
#if 1
		SmWrite32(SRAM(data_byte[0]), data0);
	       	SmWrite32(SRAM(data_byte[4]), data1);
#else
	    SmWrite8(SRAM(data_byte[0]), LOBYTE(LOWORD(data0)));
	    SmWrite8(SRAM(data_byte[1]), HIBYTE(LOWORD(data0)));
	    SmWrite8(SRAM(data_byte[2]), LOBYTE(HIWORD(data0)));
	    SmWrite8(SRAM(data_byte[3]), HIBYTE(HIWORD(data0)));
	    SmWrite8(SRAM(data_byte[4]), LOBYTE(LOWORD(data1)));
	    SmWrite8(SRAM(data_byte[5]), HIBYTE(LOWORD(data1)));
	    SmWrite8(SRAM(data_byte[6]), LOBYTE(HIWORD(data1)));
	    SmWrite8(SRAM(data_byte[7]), HIBYTE(HIWORD(data1)));
#endif
	    /* Interrupt 8051. */
	    SmWrite32(U8051_CPU_PROTOCOL_INTERRUPT, 1);

	    /* Wait for command to start. */
	    i = 10000;
	    while (SmRead8(SRAM(command_busy)) == 0)
	    {
		if (i-- <= 0)
		{
//		    KWARNING("firmware did not accept command %u\n", command);
		    break;
		}
		udelay(1);
	    }

	    /* Wait for command to finish. */
	    while (SmRead8(SRAM(command_busy)) == 1)
		udelay(1);

	    /* Return status byte. */
		
/*		tmp = SmRead8(SRAM(status_byte));
		KDEBUG("s %08x\n",  tmp);*/

	    return(SmRead8(SRAM(status_byte)) == 0x1);
	}

	/* -------------------------------------------------------------------------- */

	static void write_codec(struct ac97_codec* codec, u8 addr, u16 data)
	{
	    sm501Ptr s = (sm501Ptr) codec->private_data;

	    KDEBUG("write_codec: addr=%02x data=%04x\n", addr, data);

	    /* Write Codec register. */
	    if (!write_command(s, SET_GET_AC97_REGISTER, (unsigned int) addr << 12,
			       (unsigned int) data << 4) && (addr != AC97_RESET))
	    {
			unsigned int read_back;
			read_back = MAKEDWORD(MAKEWORD(SmRead8(SRAM(data_byte[4])),
						       SmRead8(SRAM(data_byte[5]))),
					      MAKEWORD(SmRead8(SRAM(data_byte[6])),
						       SmRead8(SRAM(data_byte[7])))) >> 4;
			KDEBUG("write_codec failed: addr=%02x data=%04x read_back=%04x\n",
				 addr, data, read_back);
	    }
	}

	static void write_codec_wrapper(struct ac97_codec* codec, u8 addr, u16 data)
	{
	    sm501Ptr s = (sm501Ptr) codec->private_data;
	    unsigned long flags;
	    int i;
#if 0
           int regv[]={1,2,4,6,0xa,0xc,0xe,0x10,0x12,0x14,0x16,0x1c};
/*
		for  (i=0;i<12;i++)
			if (regv[i]==addr)
			return;
*/
			
	    if (i_flag)	{
		for  (i=0;i<12;i++)
			if (regv[i]==addr)
			{
			i_flag--;
			return;
			}
		}
	    else{
		for  (i=0;i<12;i++)
			if (regv[i]==addr)
			i_flag=15;
	    }
	    
#endif

	    KDEBUG("write_codec_wrapper: addr=%02x data=%04x\n", addr, data);
	    spin_lock_irqsave(&s->lock, flags);

	    write_codec(codec, addr, data);

	    spin_unlock_irqrestore(&s->lock, flags);
		
	}

	static u16 read_codec(struct ac97_codec* codec, u8 addr)
	{
	    sm501Ptr s = (sm501Ptr) codec->private_data;
	    unsigned int data;
	    int i;

	    KDEBUG("read_codec: addr=%02x\n", addr);

 
	    /* Read Codec register. */
	    if (write_command(s, SET_GET_AC97_REGISTER,
			      (unsigned int) (addr << 12) | (1 << 19), 0))
	    {
		data = MAKEDWORD(MAKEWORD(SmRead8(SRAM(data_byte[4])),
					  SmRead8(SRAM(data_byte[5]))),
				 MAKEWORD(SmRead8(SRAM(data_byte[6])),
					  SmRead8(SRAM(data_byte[7])))) >> 4;
	    }
	    else
	    {
		/* Read register from SRAM in case link failes. */
		for (i = 1000; i > 0; i--)
		{
		    data = SmRead16(SRAM(ac97_regs[addr >> 1]));
		    if (data != 0xFFFF)
			break;

		    mdelay(1);
		}

		KDEBUG("read_codec failed: addr=%02x sram=%04x\n", addr, data);
	    }

	    KDEBUG("--read_codec: data=%04x\n", data);
	    return((unsigned short) data);
	}

	static u16 read_codec_wrapper(struct ac97_codec* codec, u8 addr)
		{
		    sm501Ptr s = (sm501Ptr) codec->private_data;
		    unsigned long flags;
		    unsigned int data;

		    if (addr== AC97_RECORD_SELECT)
			return 0;
			 
		    KDEBUG("read_codec_wrapper: addr=%02x\n", addr);
		    spin_lock_irqsave(&s->lock, flags);

		    data = read_codec(codec, addr);

		    spin_unlock_irqrestore(&s->lock, flags);
		    return((unsigned short) data);
		}

		static void wait_codec(struct ac97_codec* codec)
		{
		    sm501Ptr s = (sm501Ptr) codec->private_data;
		    unsigned long flags;
		    int i;
		    unsigned int data;

		    KDEBUG("wait_codec\n");
		    spin_lock_irqsave(&s->lock, flags);

		    /* Wait for AC97 active. */
		    for (i = 1000; i > 0; i--)
		    {
			data = read_codec(codec, AC97_POWER_CONTROL);
			if (data & AC97_PWR_DAC)
			{
			    KDEBUG("AC97_POWER_CONTROL: %04x\n", data);
			    break;
			}
		    }
		    if (i <= 0)
			KWARNING("AC97 codec did not respond to reset: %04x\n", data);

		    spin_unlock_irqrestore(&s->lock, flags);
		}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
/**
	This routine is provided by linux 2.6.21 but not with 2.6.28
	
 *	ac97_set_adc_rate	-	set codec rate adaption
 *	@codec: ac97 code
 *	@rate: rate in hertz
 *
 *	Set the ADC rate. Assumes the codec supports VRA. The caller is
 *	expected to have checked this little detail.
 */

unsigned int ac97_set_adc_rate(struct ac97_codec *codec, unsigned int rate)
{
	unsigned int new_rate = rate;
	u32 dacp;

	if(rate != codec->codec_read(codec, AC97_PCM_LR_ADC_RATE))
	{
		/* Power down the ADC */
		dacp=codec->codec_read(codec, AC97_POWER_CONTROL);
		codec->codec_write(codec, AC97_POWER_CONTROL, dacp|0x0100);
		/* Load the rate and read the effective rate */
		codec->codec_write(codec, AC97_PCM_LR_ADC_RATE, rate);
		new_rate=codec->codec_read(codec, AC97_PCM_LR_ADC_RATE);
		/* Power it back up */
		codec->codec_write(codec, AC97_POWER_CONTROL, dacp);
	}
	return new_rate;
}

/**
	This routine is provided by linux 2.6.21 but not with 2.6.28
	
 *	ac97_set_dac_rate	-	set codec rate adaption
 *	@codec: ac97 code
 *	@rate: rate in hertz
 *
 *	Set the DAC rate. Assumes the codec supports VRA. The caller is
 *	expected to have checked this little detail.
 */
 
unsigned int ac97_set_dac_rate(struct ac97_codec *codec, unsigned int rate)
{
	unsigned int new_rate = rate;
	u32 dacp;
	u32 mast_vol, phone_vol, mono_vol, pcm_vol;
	u32 mute_vol = 0x8000;	/* The mute volume? */

	if(rate != codec->codec_read(codec, AC97_PCM_FRONT_DAC_RATE))
	{
		/* Mute several registers */
		mast_vol = codec->codec_read(codec, AC97_MASTER_VOL_STEREO);
		mono_vol = codec->codec_read(codec, AC97_MASTER_VOL_MONO);
		phone_vol = codec->codec_read(codec, AC97_HEADPHONE_VOL);
		pcm_vol = codec->codec_read(codec, AC97_PCMOUT_VOL);
		codec->codec_write(codec, AC97_MASTER_VOL_STEREO, mute_vol);
		codec->codec_write(codec, AC97_MASTER_VOL_MONO, mute_vol);
		codec->codec_write(codec, AC97_HEADPHONE_VOL, mute_vol);
		codec->codec_write(codec, AC97_PCMOUT_VOL, mute_vol);
		
		/* Power down the DAC */
		dacp=codec->codec_read(codec, AC97_POWER_CONTROL);
		codec->codec_write(codec, AC97_POWER_CONTROL, dacp|0x0200);
		/* Load the rate and read the effective rate */
		codec->codec_write(codec, AC97_PCM_FRONT_DAC_RATE, rate);
		new_rate=codec->codec_read(codec, AC97_PCM_FRONT_DAC_RATE);
		/* Power it back up */
		codec->codec_write(codec, AC97_POWER_CONTROL, dacp);

		/* Restore volumes */
		codec->codec_write(codec, AC97_MASTER_VOL_STEREO, mast_vol);
		codec->codec_write(codec, AC97_MASTER_VOL_MONO, mono_vol);
		codec->codec_write(codec, AC97_HEADPHONE_VOL, phone_vol);
		codec->codec_write(codec, AC97_PCMOUT_VOL, pcm_vol);
	}
	return new_rate;
}
#endif
static void set_recording_rate(sm501Ptr s, unsigned rate)
{

		   if (!s->recording.vbr)
			s->recording.rate = 48000;
		    else
		    {
			if (rate > 48000)
			    rate = 48000;
			else if (rate < 11500)
			    rate = 11500;

			KDEBUG("Set recording rate to %d\n", rate);
			s->recording.rate = ac97_set_adc_rate(s->codec, rate);
			KDEBUG("-> %d\n", s->recording.rate);
		    }
}

static void set_playing_rate(sm501Ptr s, unsigned rate)
{
		    if (!s->playing.vbr)
			s->playing.rate = 48000;
		    else
		    {
			if (rate > 48000)
			    rate = 48000;
			if (rate < 4000)
			    rate = 4000;

			KDEBUG("Set playing rate to %d\n", rate);
			s->playing.rate = ac97_set_dac_rate(s->codec, rate);
			KDEBUG("-> %d\n", s->playing.rate);
		    }
}


/* -------------------------------------------------------------------------- */

		static void select_buffer(sm501Ptr s, unsigned char status)
		{
			if (status & 1)
			{
			    s->playing.hw_buffer = SRAM(A.B.playback_buffer_0);
			    s->playing.hw_buffer_size = SRAM_SIZE(A.B.playback_buffer_0);
			}
			else  if (status & 2)
			{
			    s->playing.hw_buffer = SRAM(A.B.playback_buffer_1);
			    s->playing.hw_buffer_size = SRAM_SIZE(A.B.playback_buffer_1);
			}
			else if (status & 0x10)
			{
			    s->recording.hw_buffer = SRAM(A.C.capture_buffer_0);
			    s->recording.hw_buffer_size = SRAM_SIZE(A.C.capture_buffer_0);
			}
			else if (status & 0x20)
			{
			    s->recording.hw_buffer = SRAM(A.C.capture_buffer_1);
			    s->recording.hw_buffer_size = SRAM_SIZE(A.C.capture_buffer_1);
			}
		}

		#define U8_TO_S16(v)    MAKEWORD(0, v)  //(v) - 0x80)

		static void fill_playing(sm501Ptr s)
		{
		    unsigned int sample;
		    unsigned hw_buffer;
		    int hw_buffer_count, count;
		    unsigned hw_ptr, sw_buffer_size, total_bytes;

		    /* Load parameters. */
		    hw_buffer = s->playing.hw_buffer;
		    hw_buffer_count = s->playing.hw_buffer_size;
		    count = s->playing.count;

		    KDEBUG("sample count:%d\n",count);

		    /* Test for buffer underrun. */
		    if (count <= 0)
		    {
			/* Buffer underrun. */
			s->playing.error++;
			KDEBUG("Buffer underrun count:%d\n",count );
		    }
		    else
		    {
			sw_buffer_size = s->playing.sw_buffer_size;
			hw_ptr = s->playing.hw_ptr;
			total_bytes = s->playing.total_bytes;

			/* Loop while hardware buffer has room. */
			while (hw_buffer_count > 0)
			{
			    /* Determine sample bit size. */
			    if (s->playing.bits == 8)
			    {
				/* Make sure software buffer has sample. */
				if (count < 1)
				    break;

				/* Convert sample into 24-bit. */
				sample = U8_TO_S16(s->playing.sw_buffer[hw_ptr]);

				/* Update software buffer. */
				hw_ptr += 1;
				count -= 1;
				total_bytes += 1;
			    }
			    else
			    {
				/* Make sure software buffer has sample. */
				if (count < 2)
				    break;

				/* Convert sample into 24-bit. */
				sample = MAKEWORD(s->playing.sw_buffer[hw_ptr + 0],
						  s->playing.sw_buffer[hw_ptr + 1]);

				/* Update software buffer. */
				hw_ptr += 2;
				count -= 2;
				total_bytes += 2;
			    }

			    hw_ptr %= sw_buffer_size;

			    /* Store sample in hardware buffer. */
			    SmWrite32(hw_buffer, sample << 4);
		#ifdef CONFIG_SOUND_SM501_DEBUG
			    if (s->sample_index < s->sample_size)
				s->samples[s->sample_index++] = sample << 4;
		#endif
			    hw_buffer += 4;
			    hw_buffer_count -= 4;

			    /* Test if sample is stereo. */
			    if (s->playing.channels == 2)
			    {

				
				/* Determine sample bit size. */
				if (s->playing.bits == 8)
				{
				    /* Make sure software buffer has sample. */
				    if (count < 1)
				    {
					/* Stereo data missing - error. */
					s->playing.error++;
					KDEBUG("Stereo data missing 1\n");

					break;
				    }

				    /* Convert sample into 24-bit. */
				    sample = U8_TO_S16(s->playing.sw_buffer[hw_ptr]);

				    /* Update software buffer. */
				    hw_ptr += 1;
				    count -= 1;
				    total_bytes += 1;
				}
				else
				{
				    /* Make sure software buffer has sample. */
				    if (count < 2)
				    {
					/* Stereo data missing - error. */
					s->playing.error++;
					KDEBUG("Stereo data missing 2\n");
					break;
				    }

				    /* Convert sample into 24-bit. */
				    sample = MAKEWORD(s->playing.sw_buffer[hw_ptr + 0],
						      s->playing.sw_buffer[hw_ptr + 1]);

				    /* Update software buffer. */
				    hw_ptr += 2;
				    count -= 2;
				    total_bytes += 2;
				}


				hw_ptr %= sw_buffer_size;
			    }

			    /* Store sample in hardware buffer. */
			    SmWrite32(hw_buffer, sample << 4);
		#ifdef CONFIG_SOUND_SM501_DEBUG
			    if (s->sample_index < s->sample_size)
				s->samples[s->sample_index++] = sample << 4;
		#endif
			    hw_buffer += 4;
			    hw_buffer_count -= 4;
			}

			/* Store software buffer parameters. */
			s->playing.hw_ptr = hw_ptr % sw_buffer_size;
			s->playing.count = count;
			s->playing.total_bytes = total_bytes;
		    }

		    /* See if hardware buffer needs to be cleared. */
		    if (hw_buffer_count > 0)
		    {
			/* Mark clear flag. */
			s->playing.endcleared = 1;

			/* Fill hardware buffer with samples. */
			KDEBUG("clearing %d samples\n", hw_buffer_count / 4);
			while (hw_buffer_count > 0)
			{
			    SmWrite32(hw_buffer, 0);
		#ifdef CONFIG_SOUND_SM501_DEBUG
			    if (s->sample_index < s->sample_size)
				s->samples[s->sample_index++] = 0;
		#endif
			    hw_buffer += 4;
			    hw_buffer_count -= 4;
			}
		    }
		}

		#define S16_TO_U8(v) ((unsigned char) (HIBYTE(v) + 0x80))

		static void get_recording(sm501Ptr s)
		{

/*				hw_ptr:  hwbuffer   pos
				count:    copy  count
				total_bytes:   bytes in sw buffer
*/
		    unsigned int sample;
		    unsigned hw_buffer;
		    int hw_buffer_size, count;
		    unsigned hw_ptr, sw_buffer_size, total_bytes;
		    static int cc;
		
		    /* Get parameters. */
		    hw_buffer = s->recording.hw_buffer;
		    hw_buffer_size = s->recording.hw_buffer_size;
		    sw_buffer_size = s->recording.sw_buffer_size;
		    count = s->recording.count;

		    KDEBUG("get_recording NO. %d \n", cc++);
			
		    /* Test for buffer overrun. */
		    if (count >= sw_buffer_size)
		    {
			KDEBUG(" Buffer overrun 1. \n");
			s->recording.error++;
		    }
		    else
		    {
			hw_ptr = s->recording.hw_ptr;
			total_bytes = s->recording.total_bytes;

			/* Loop while hardware buffer has samples. */
			while (hw_buffer_size > 0)
			{
			    /* Read sample from hardware buffer. */
			    sample = SmRead32(hw_buffer);
			    hw_buffer += 4;
			    hw_buffer_size -= 4;

				
			// first channel
                         {
			    /* Test for buffer overrun. */
			    if (count >= sw_buffer_size)
			    {
				KDEBUG(" Buffer overrun 2\n ");
				s->recording.error++;
				break;
			    }

			    KDEBUG("sample:%08x\n ", (sample) );
				
			    /* Determine sample bit size. */
			    if (s->recording.bits == 8)
			    {
				/* Convert sample into 8-bit. */
				s->recording.sw_buffer[hw_ptr] = 
						(unsigned char) (sample >> 12);
			    KDEBUG("%8x\n ",s->recording.sw_buffer[hw_ptr]  );


				/* Update hardware buffer. */
				hw_ptr += 1;        //hwbuffer   pos
				count += 1;          // copy  count
				total_bytes += 1;    //bytes in sw buffer
			    }
			    else
			    {
				/* Convert sample into 16-bit. */
				s->recording.sw_buffer[hw_ptr + 0] = (unsigned char)
								     (sample >> 4);
				s->recording.sw_buffer[hw_ptr + 1] = (unsigned char)
								     (sample >> 12);

				/* Update hardware buffer. */
				hw_ptr += 2;
				count += 2;
				total_bytes += 2;
			    }

			    hw_ptr %= sw_buffer_size;

			    /* Read sample from hardware buffer. */
			    sample = SmRead32(hw_buffer);
			    hw_buffer += 4;
			    hw_buffer_size -= 4;
	  		   }


			
			    /* second channel. */
			    if (s->recording.channels == 2)
			   {
				/* Test for buffer overrun. */
				if (count >= sw_buffer_size)
				{
				KDEBUG(" Buffer overrun 3. \n");
				    s->recording.error++;
				    break;
				}

				/* Determine sample bit size. */
				if (s->recording.bits == 8)
				{
				    /* Convert sample into 8-bit. */
				    s->recording.sw_buffer[hw_ptr] = 
						    (unsigned char) (sample >> 12);

				    /* Update hardware buffer. */
				    hw_ptr += 1;
				    count += 1;
				    total_bytes += 1;
				}
				else
				{
				    /* Convert sample into 16-bit. */
				    s->recording.sw_buffer[hw_ptr + 0] = (unsigned char)
									 (sample >> 4);
				    s->recording.sw_buffer[hw_ptr + 1] = (unsigned char)
									 (sample >> 12);

				    /* Update hardware buffer. */
				    hw_ptr += 2;
				    count += 2;
				    total_bytes += 2;
				}
				hw_ptr %= sw_buffer_size;
			    } 


				
			}

			/* Store hardware buffer parameters. */
			s->recording.hw_ptr = hw_ptr % sw_buffer_size;
			s->recording.count = count;
			s->recording.total_bytes = total_bytes;
		    }
		}

		/* -------------------------------------------------------------------------- */

		static void start_playing(sm501Ptr s, unsigned short SampleRate )
		{
		    unsigned long flags;
			
		    if (SampleRate)	
                  write_codec_wrapper(s->codec, AC97_PCM_FRONT_DAC_RATE, SampleRate);
				  
		    spin_lock_irqsave(&s->lock, flags);
		    if (!s->playing.running && s->playing.ready && s->playing.count > 0)
		    {
		#ifdef CONFIG_SOUND_SM501_DEBUG
			KDEBUG("start playing: %d bits, %d channels\n", s->playing.bits,
			       s->playing.channels);
			s->sample_index = 0;
		#endif
			fill_playing(s);
			write_command(s, START_STOP_AUDIO_PLAYBACK, s->playing.running = 1, 0);
		    }
		    spin_unlock_irqrestore(&s->lock, flags);
		}

		static inline void stop_playing(sm501Ptr s)
		{
		    unsigned long flags;
		    unsigned char status;

		    spin_lock_irqsave(&s->lock, flags);
		    write_command(s, START_STOP_AUDIO_PLAYBACK, s->playing.running = 0, 0);
		    spin_unlock_irqrestore(&s->lock, flags);
		    status = SmRead8(SRAM(buffer_status));
		    SmWrite8(SRAM(buffer_status), status&0xF0);
		}

		static void start_recording(sm501Ptr s, unsigned short SampleRate )
		{
		    unsigned long flags;

		    if (SampleRate)	{
			write_codec_wrapper(s->codec, AC97_PCM_LR_ADC_RATE, SampleRate);  //in case of line-in 
			write_codec_wrapper(s->codec, AC97_PCM_MIC_ADC_RATE, SampleRate);				  
		    }
		    spin_lock_irqsave(&s->lock, flags);
		    if (!s->recording.running && s->recording.ready &&
			s->recording.count < s->recording.sw_buffer_size)
		    {
			KDEBUG("start recording: %d bits, %d channels\n", s->recording.bits,
			       s->recording.channels);
			write_command(s, START_STOP_AUDIO_CAPTURE, s->recording.running = 1, 0);
		    }
		    spin_unlock_irqrestore(&s->lock, flags);
		    mdelay(2);

		}

		static inline void stop_recording(sm501Ptr s)
		{
		    unsigned long flags;
		    unsigned char status;

		    spin_lock_irqsave(&s->lock, flags);
		    write_command(s, START_STOP_AUDIO_CAPTURE, s->recording.running = 0, 0);
		    spin_unlock_irqrestore(&s->lock, flags);
		    status = SmRead8(SRAM(buffer_status));
		    SmWrite8(SRAM(buffer_status), status&0x0F);			
		}

		/* -------------------------------------------------------------------------- */

		static void dealloc_buffer(sm501Ptr s, sm501BufferPtr buffer)
		{
		    /* Free allocated software buffer. */
		    if (buffer->sw_buffer != NULL)
			kfree(buffer->sw_buffer);

		    /* Disable buffer. */
		    buffer->sw_buffer = NULL;
		    buffer->enabled = buffer->ready = 0;
		}

		static int program_buffer(sm501Ptr s, sm501BufferPtr buffer, unsigned hw_buffer,
					  unsigned hw_buffer_size)
		{
		    /* Zero offsets, counters, and flags. */
		    buffer->hw_ptr = buffer->sw_ptr = 0;
		    buffer->total_bytes = buffer->count = buffer->error = 0;
		    buffer->endcleared = 0;

		    if (buffer->sw_buffer == NULL)
		    {
			/* Allocate software buffer. */
			buffer->sw_buffer = kmalloc(buffer->sw_buffer_size = 0x4000, GFP_KERNEL);
			if (buffer->sw_buffer == NULL){
			    KDEBUG("Not enough memory.");
			    return(-ENOMEM);
			}
		    }

		    /* Set hardware buffer. */
		    buffer->hw_buffer = hw_buffer;
		    buffer->hw_buffer_size = hw_buffer_size;

		    /* Calculate threshold(count of int size). */
		    buffer->threshold = hw_buffer_size / 4 * (buffer->bits >> 3);

		    /* Enable buffer. */
		    buffer->enabled = buffer->ready = 1;
		    return(0);
		}

		static inline int program_recording(sm501Ptr s)
		{
		    stop_recording(s);
		    return(program_buffer(s, &s->recording, SRAM(A.C.capture_buffer_0),
					  SRAM_SIZE(A.C.capture_buffer_0)));
		}

		static inline int program_playing(sm501Ptr s)
		{
		    stop_playing(s);
		    return(program_buffer(s, &s->playing, SRAM(A.B.playback_buffer_0),
					  SRAM_SIZE(A.B.playback_buffer_0)));
		}

		static irqreturn_t sm501_interrupt(int irq, void* dev_id, struct pt_regs* regs)
		{
		    sm501Ptr s = (sm501Ptr) dev_id;
		    unsigned int status, mask;
                  unsigned long flags;

		    /* Read interrupt status. */
		    status = SmRead32(INTERRUPT_STATUS);
		    if (TEST_FIELD(status, INTERRUPT_STATUS_8051, NOT_ACTIVE))
			return IRQ_NONE; 

//		    KDEBUG("interrupt status: %08x\n", status);

		    /* Reset 8051 interrupt status. */
			
		    mask = SmRead32(INTERRUPT_MASK);
		    SmWrite32(INTERRUPT_MASK, SET_FIELD(mask, INTERRUPT_MASK_8051, DISABLE));	
			
		    spin_lock_irqsave(&s->lock, flags);
		    SmRead32(U8051_8051_PROTOCOL_INTERRUPT);
		    /* Check 8051 buffer status. */
		    status = SmRead8(SRAM(buffer_status));
//		    KDEBUG("interrupt status: %08x\n", status);
		    if (status != 0)
		    {
			select_buffer(s, status);
			if (status & 0x0F)
			{
				// Playback buffer empty.
			    fill_playing(s);
			    if (s->playing.error)
			    {
				KDEBUG("playing underflow.\n");
				write_command(s, START_STOP_AUDIO_PLAYBACK,
					      s->playing.running = 0, 0);
				s->playing.error = 0;
			    }
			    else if (s->playing.sw_buffer_size - s->playing.count >= s->playing.threshold)
			    {
				wake_up(&s->playing.wait);
			    }
			}
			if (status & 0xF0)
			{
				// Recording buffer full.
			    get_recording(s);
			    if (s->recording.error)
			    {
				KDEBUG("recording overflow.\n");
				write_command(s, START_STOP_AUDIO_CAPTURE,
					      s->recording.running = 0, 0);
				s->recording.error = 0;
			    }
			    else if (s->recording.count >= s->recording.threshold)
			    {
				wake_up(&s->recording.wait);
			    }
			}
			
		    }
		    spin_unlock_irqrestore(&s->lock, flags);
			
		    SmWrite32(INTERRUPT_MASK, SET_FIELD(mask, INTERRUPT_MASK_8051, ENABLE));	

		    return IRQ_HANDLED;
		}

		/* -------------------------------------------------------------------------- */

		static int sm501_open_mixdev(struct inode* inode, struct file* file)
		{
		    int minor;
		    sm501Ptr s;
		    KDEBUG("sm501 open mixdev\n");

		    minor = MINOR(inode->i_rdev);
		    //minor = iminor(inode);
		    s = host_data;
		    VALIDATE_STATE(s);

		    if (s->codec->dev_mixer != minor)
			return(-ENODEV);

		    file->private_data = s;
		    return(0);
		}

		static int sm501_release_mixdev(struct inode* inode, struct file* file)
		{
		    host_data = (sm501Ptr) file->private_data;
		    VALIDATE_STATE(host_data);

		    return(0);
		}

		static int sm501_ioctl_mixdev(struct inode* inode, struct file* file,
					      unsigned int cmd, unsigned long arg)
		{
		    sm501Ptr s = (sm501Ptr) file->private_data;
		    VALIDATE_STATE(s);

		    return(s->codec->mixer_ioctl(s->codec, cmd, arg));
		}

		static struct file_operations sm501_mixer_fops =
		{
		    .owner=      THIS_MODULE,
		    .llseek=     no_llseek,
		    .ioctl=      sm501_ioctl_mixdev,
		    .open=       sm501_open_mixdev,
		    .release=    sm501_release_mixdev,
		};

		/* -------------------------------------------------------------------------- */

		static int drain_playing(sm501Ptr s, int nonblock)
		{
		    DECLARE_WAITQUEUE(wait, current);
		    unsigned long flags;
		    int count;

		    add_wait_queue(&s->playing.wait, &wait);

		    while (1)
		    {
			__set_current_state(TASK_UNINTERRUPTIBLE);
			spin_lock_irqsave(&s->lock, flags);
			count = s->playing.count;
			spin_unlock_irqrestore(&s->lock, flags);
			if (count <= 0)
			    break;

			if (signal_pending(current))
			    break;

			if (nonblock)
			{
			    remove_wait_queue(&s->playing.wait, &wait);
			    set_current_state(TASK_RUNNING);
			    return(-EBUSY);
			}

			if (!schedule_timeout(count * HZ / s->playing.rate))
			    KDEBUG("drain_playing: can not schedule a timeout\n");
		    }

		    remove_wait_queue(&s->playing.wait, &wait);
		    set_current_state(TASK_RUNNING);
		    if (signal_pending(current))
			return(-ERESTARTSYS);

		    return(0);
		}

/* -------------------------------------------------------------------------- */

static ssize_t sm501_oss_read(struct file* file, char* buffer, size_t count,  loff_t* ppos)
		{
		    sm501Ptr s;
		    ssize_t ret;
		    unsigned long flags;
		    unsigned sw_ptr;
		    int byte_count;
		    char* pbuf=buffer;

		    KDEBUG("sm501_read: count=%d, ppos=%ld, file->f_pos=%ld\n", count, ppos, &file->f_pos);

		    s = (sm501Ptr) file->private_data;
		    DECLARE_WAITQUEUE(wait, current);
		    ret = 0;

		    VALIDATE_STATE(s);

		    /* Verify file position and file buffer. */
/*			
		   if (ppos != &file->f_pos)
				return(-ESPIPE);
*/		   
		    if (!access_ok(VERIFY_WRITE, buffer, count))
			return(-EFAULT);

		    /* Down semaphore. */
		    down(&s->sem);
		    if (!s->recording.ready)
		    {
			/* Program recording buffer. */
			ret = program_recording(s);
			if (ret != 0)
			    goto out_set_state;
		    }
		    add_wait_queue(&s->recording.wait, &wait);

		    /* Loop while there are bytes to be read. */
		    while (count > 0)
		    {
			/* Get number of bytes left in software buffer. */
			spin_lock_irqsave(&s->lock, flags);
			
		       /*sw_ptr   current position in buffer*/
			sw_ptr = s->recording.sw_ptr;        
			byte_count = s->recording.sw_buffer_size - sw_ptr;
			
			/*error check*/
			if (byte_count > s->recording.count)
			    byte_count = s->recording.count;
			if (byte_count <= 0)
			    __set_current_state(TASK_INTERRUPTIBLE);
			spin_unlock_irqrestore(&s->lock, flags);

		    KDEBUG("sw_ptr=%d, sw_buffer_size=%d, s->recording.count=%d s->recording.threshold=%d \n", sw_ptr, s->recording.sw_buffer_size, s->recording.count, s->recording.threshold);
			/* Don't overflow file buffer. */
			if (byte_count > count)
			    byte_count = count;
			if (byte_count <= 0)
			{
			    /* Start recording. */
			    if (s->recording.enabled)
				start_recording(s,0);

			/* For non-blocking I/O, return now. */
			    if (file->f_flags & O_NONBLOCK)
			    {
				if (ret != 0)
				    ret = -EAGAIN;
				goto out_up;
			    }

		    KDEBUG("sm501_read: 5\n");
			/* Up semaphore and reschedule. */
			    up(&s->sem);
			    schedule();
			    if (signal_pending(current))
			    {
				if (ret != 0)
				    ret = -ERESTARTSYS;
				goto out_remove_queue;
			    }

			    /* Down semaphore again. */
			    down(&s->sem);
			}
			else
			{
		    KDEBUG("sm501_read: 5.1\n");
			
			    /* Copy bytes from software buffer to file buffer. */
			    //if (copy_to_user(buffer, s->recording.sw_buffer + sw_ptr,
			    if (copy_to_user(pbuf, s->recording.sw_buffer + sw_ptr,
					     byte_count))
			    {
				if (ret != 0)
				    ret = -EFAULT;
				goto out_up;
			    }

		    KDEBUG("sm501_read: 6\n");
			    /* Update software buffer. */
			    sw_ptr = (sw_ptr + byte_count) % s->recording.sw_buffer_size;
			    spin_lock_irqsave(&s->lock, flags);
			    s->recording.sw_ptr = sw_ptr;
			    s->recording.count -= byte_count;
			    spin_unlock_irqrestore(&s->lock, flags);

			    /* Update file buffer. */
			    count -= byte_count;
			    //buffer += byte_count;
			    pbuf += byte_count;
			    ret += byte_count;

			    /* Start recording if it was stopped. */
			    if (s->recording.enabled)
				start_recording(s,0);
			}
		    }

		out_up:
		    /* Up semaphore. */
		    up(&s->sem);
		out_remove_queue:
		    remove_wait_queue(&s->recording.wait, &wait);
		out_set_state:
		    set_current_state(TASK_RUNNING);

		    KDEBUG("sm501_read: 7\n");
		    /* Return error or number of bytes read. */
		    return(ret);
		    //return(nonseekable_open(inode, file));
		}

static ssize_t sm501_oss_write(struct file* file, const char* buffer, size_t count, loff_t* ppos)
		{
		    sm501Ptr s = (sm501Ptr) file->private_data;
		    wait_queue_t wait = __WAITQUEUE_INITIALIZER(wait, current);
		    //DECLARE_WAITQUEUE(wait, current);
		    ssize_t ret = 0;
		    unsigned long flags;
		    unsigned sw_ptr;
		    int byte_count;

		    VALIDATE_STATE(s);

//		    KDEBUG("sm501_write: buffer=%d, count=%d\n", buffer, count);
		    /* Verify file position and file buffer. */
		    //if (ppos != &file->f_pos)
		    //	return(-ESPIPE);
		
		    if (!access_ok(VERIFY_READ, buffer, count))
			return(-EFAULT);

		    /* Down semaphore. */
		    down(&s->sem);
		    if (!s->playing.ready)
		    {
			/* Program playing buffer. */
			ret = program_playing(s);
			if (ret != 0)
			    goto out_set_state;
		    }
		    add_wait_queue(&s->playing.wait, &wait);

		    /* Loop while bytes in file buffer. */
		    while (count > 0)
		    {
			/* Get number of bytes left in software buffer. */
			spin_lock_irqsave(&s->lock, flags);
			if (s->playing.count < 0)
			{
			    /* Reset in case of error. */
			    s->playing.count = 0;
			    s->playing.sw_ptr = s->playing.hw_ptr;
			}
			sw_ptr = s->playing.sw_ptr;
			byte_count = s->playing.sw_buffer_size - sw_ptr;
			if (s->playing.count + byte_count > s->playing.sw_buffer_size)
			    byte_count = s->playing.sw_buffer_size - s->playing.count;
			if (byte_count <= 0)
			    __set_current_state(TASK_INTERRUPTIBLE);
			spin_unlock_irqrestore(&s->lock, flags);

			/* Don't overflow file buffer. */
			if (byte_count > count)
			    byte_count = count;

			if (byte_count <= 0)
			{
			    /* Start playing. */
			    if (s->playing.enabled)
				start_playing(s,0);

			    /* For non-blocking I/O, return now. */
			    if (file->f_flags & O_NONBLOCK)
			    {
				if (ret != 0)
				    ret = -EAGAIN;
				goto out_up;
			    }

			    /* Up semaphore and reschedule. */
			    up(&s->sem);
			    schedule();
			    if (signal_pending(current))
			    {
				if (ret != 0)
				    ret = -ERESTARTSYS;
				goto out_remove_queue;
			    }

			    /* Down semaphore again. */
			    down(&s->sem);
			}
			else
			{
			    /* Copy bytes from file buffer to software buffer. */
			    if (copy_from_user(s->playing.sw_buffer + sw_ptr, buffer,
					       byte_count))
			    {
				if (ret != 0)
				    ret = -EFAULT;
				goto out_up;
			    }

		#ifdef CONFIG_SOUND_SM501_DEBUG
		{
			    int n = byte_count;
			    if (n >= s->data_size - s->data_index)
				n = s->data_size - s->data_index;
			    copy_from_user(s->data + s->data_index, buffer, n);
			    s->data_index += n;
		}
		#endif

			    /* Update software buffer. */
			    sw_ptr = (sw_ptr + byte_count) % s->playing.sw_buffer_size;
			    spin_lock_irqsave(&s->lock, flags);
			    s->playing.sw_ptr = sw_ptr;
			    s->playing.count += byte_count;
			    s->playing.endcleared = 0;
			    spin_unlock_irqrestore(&s->lock, flags);

			    /* Update file buffer. */
			    count -= byte_count;
			    buffer += byte_count;
			    ret += byte_count;

			    /* Start playing if it was stopped. */
			    if (s->playing.enabled)
				start_playing(s,0);
			}
		    }

		out_up:
		    /* Up semaphore. */
		    up(&s->sem);
		out_remove_queue:
		    remove_wait_queue(&s->playing.wait, &wait);
		out_set_state:
		    set_current_state(TASK_RUNNING);

		    /* Return error or number of bytes written. */
		    return(ret);
		}

		static unsigned int sm501_oss_poll(struct file* file,
					       struct poll_table_struct* wait)
		{
		    sm501Ptr s = (sm501Ptr) file->private_data;
		    unsigned long flags;
		    unsigned int mask = 0;

		    VALIDATE_STATE(s);

		    if (file->f_mode & FMODE_WRITE)
		    {
			if (!s->playing.ready && program_playing(s))
			    return(0);
			poll_wait(file, &s->playing.wait, wait);
		    }
		    if (file->f_mode & FMODE_READ)
		    {
			if (!s->recording.ready && program_recording(s))
			    return(0);
			poll_wait(file, &s->recording.wait, wait);
		    }

		    spin_lock_irqsave(&s->lock, flags);
		    if (file->f_mode & FMODE_READ)
		    {
			if (s->recording.count >= s->recording.threshold)
			    mask |= POLLIN | POLLRDNORM;
		    }
		    if (file->f_mode & FMODE_WRITE)
		    {
			if (s->playing.sw_buffer_size - s->playing.count >=  s->playing.threshold)
			    mask |= POLLOUT | POLLWRNORM;
		    }
		    spin_unlock_irqrestore(&s->lock, flags);
		    return(mask);
		}

		static int sm501_oss_ioctl(struct inode* inode, struct file* file, unsigned int cmd,
				       unsigned long arg)
		{
		    sm501Ptr s = (sm501Ptr) file->private_data;
		    unsigned long flags;
		    audio_buf_info abinfo;
		    count_info cinfo;
		    int count, val, ret;

		    VALIDATE_STATE(s);

		    KDEBUG("sm501_ioctl:  -> %d\n", cmd);

		    switch (cmd)
		    {
		    case OSS_GETVERSION:
			return(put_user(SOUND_VERSION, (int*) arg));

		    case SNDCTL_DSP_SYNC:
			KDEBUG("sm501_ioctl: SNDCTL_DSP_SYNC\n");
			if (file->f_mode & FMODE_WRITE)
			    return(drain_playing(s, 0));
			return(0);

		    case SNDCTL_DSP_SETDUPLEX:
//			return 0;
			return(-EINVAL);

		    case SNDCTL_DSP_GETCAPS:
			return(put_user(DSP_CAP_REALTIME | DSP_CAP_TRIGGER, (int*) arg));

		    case SNDCTL_DSP_RESET:
			KDEBUG("sm501_ioctl: SNDCTL_DSP_RESET\n");
			if (file->f_mode & FMODE_WRITE)
			{
			    stop_playing(s);
			    synchronize_irq(s->irq);
			    s->playing.sw_ptr = s->playing.hw_ptr = 0;
			    s->playing.count = s->playing.total_bytes = 0;
			}
			if (file->f_mode & FMODE_READ)
			{
			    stop_recording(s);
			    synchronize_irq(s->irq);
			    s->recording.sw_ptr = s->recording.hw_ptr = 0;
			    s->recording.count = s->recording.total_bytes = 0;
			}

			return(0);

		    case SNDCTL_DSP_SPEED:
			if (get_user(val, (int*) arg))
			    return(-EFAULT);
			
			KDEBUG("sm501_ioctl: SNDCTL_DSP_SPEED %d\n", val);
			if (val >= 0)
			{
			    if (file->f_mode & FMODE_READ)
			    {
				stop_recording(s);
				s->recording.ready = 0;
		#ifdef CONFIG_SOUND_SM501_VBR
				set_recording_rate(s, val);
		#endif
			    }
			    if (file->f_mode & FMODE_WRITE)
			    {
				stop_playing(s);
				s->playing.ready = 0;
		#ifdef CONFIG_SOUND_SM501_VBR
				set_playing_rate(s, val);
		#endif
			    }
			}

			return(put_user((file->f_mode & FMODE_READ) ? s->recording.rate :
					s->playing.rate, (int*) arg));

		    case SNDCTL_DSP_STEREO:
			if (get_user(val, (int*) arg))
			    return(-EFAULT);

			KDEBUG("sm501_ioctl: SNDCTL_DSP_STEREO %d\n", val);
			if (file->f_mode & FMODE_READ)
			{
			    stop_recording(s);
			    s->recording.ready = 0;
			    spin_lock_irqsave(&s->lock, flags);
			    s->recording.channels = (val != 0) ? 2 : 1;
			    spin_unlock_irqrestore(&s->lock, flags);
			}
			if (file->f_mode & FMODE_WRITE)
			{
			    stop_playing(s);
			    s->playing.ready = 0;
			    spin_lock_irqsave(&s->lock, flags);
			    s->playing.channels = (val != 0) ? 2 : 1;
			    spin_unlock_irqrestore(&s->lock, flags);
			}

			return(0);

		    case SNDCTL_DSP_CHANNELS:
			if (get_user(val, (int*) arg))
			    return(-EFAULT);

			KDEBUG("sm501_ioctl: SNDCTL_DSP_CHANNELS %d\n", val);
			if (val != 0)
			{
			    if (file->f_mode & FMODE_READ)
			    {
				stop_recording(s);
				s->recording.ready = 0;
				spin_lock_irqsave(&s->lock, flags);
				s->recording.channels = (val >= 2) ? 2 : 1;
				spin_unlock_irqrestore(&s->lock, flags);
			    }
			    if (file->f_mode & FMODE_WRITE)
			    {
				stop_playing(s);
				s->playing.ready = 0;
				spin_lock_irqsave(&s->lock, flags);
				s->playing.channels = (val >= 2) ? 2 : 1;
				spin_unlock_irqrestore(&s->lock, flags);
			    }
			}

			return(put_user((file->f_mode & FMODE_READ) ? s->recording.channels :
					s->playing.channels, (int*) arg));

		    case SNDCTL_DSP_GETFMTS:
			return(put_user(AFMT_S16_LE | AFMT_U8, (int*) arg));

		    case SNDCTL_DSP_SETFMT:
			if (get_user(val, (int*) arg))
			    return(-EFAULT);

			KDEBUG("sm501_ioctl: SNDCTL_DSP_SETFMT %d\n", val);
			if (val != AFMT_QUERY)
			{
			    if (file->f_mode & FMODE_READ)
			    {
				stop_recording(s);
				s->recording.ready = 0;
				spin_lock_irqsave(&s->lock, flags);
				s->recording.bits = (val == AFMT_S16_LE) ? 16 : 8;
				spin_unlock_irqrestore(&s->lock, flags);
			    }
			    if (file->f_mode & FMODE_WRITE)
			    {
				stop_playing(s);
				s->playing.ready = 0;
				spin_lock_irqsave(&s->lock, flags);
				s->playing.bits = (val == AFMT_S16_LE) ? 16 : 8;
				spin_unlock_irqrestore(&s->lock, flags);
			    }
			}

			return(put_user((((file->f_mode & FMODE_READ) ? s->recording.bits :
					s->playing.bits) == 16) ? AFMT_S16_LE : AFMT_U8,
					(int*) arg));

		    case SNDCTL_DSP_POST:
			return(0);

		    case SNDCTL_DSP_GETTRIGGER:
			val = 0;
			if ((file->f_mode & FMODE_READ) && s->recording.running)
			    val |= PCM_ENABLE_INPUT;
			if ((file->f_mode & FMODE_WRITE) && s->playing.running)
			    val |= PCM_ENABLE_OUTPUT;
			KDEBUG("sm501_ioctl: SNDCTL_DSP_GETTRIGGER -> 0x%x\n", val);
			return(put_user(val, (int*) arg));

		    case SNDCTL_DSP_SETTRIGGER:
			if (get_user(val, (int*) arg))
			    return(-EFAULT);

			KDEBUG("sm501_ioctl: SNDCTL_DSP_SETTRIGGER 0x%x\n", val);
			if (file->f_mode & FMODE_READ)
			{
			    if (val & PCM_ENABLE_INPUT)
			    {
				if (!s->recording.ready)
				{
				    ret = program_recording(s);
				    if (ret != 0)
					return(ret);
				}

				s->recording.enabled = 1;
				start_recording(s,0);
			    }
			    else
			    {
				s->recording.enabled = 0;
				stop_recording(s);
			    }
			}
			if (file->f_mode & FMODE_WRITE)
			{
			    if (val & PCM_ENABLE_OUTPUT)
			    {
				if (!s->playing.ready)
				{
				    ret = program_playing(s);
				    if (ret != 0)
					return(ret);
				}

				s->playing.enabled = 1;
				start_playing(s,0);
			    }
			    else
			    {
				s->playing.enabled = 0;
				stop_playing(s);
			    }
			}

			return(0);

		    case SNDCTL_DSP_GETOSPACE:
			if (!(file->f_mode & FMODE_WRITE))
			    return(-EINVAL);

			if (!s->playing.ready)
			{
			    ret = program_playing(s);
			    if (ret != 0)
				return(ret);
			}

			spin_lock_irqsave(&s->lock, flags);
			abinfo.fragsize = s->playing.threshold;
			count = s->playing.count;
			if (count < 0)
			    count = 0;
			abinfo.bytes = s->playing.sw_buffer_size - count;
			abinfo.fragstotal = s->playing.sw_buffer_size / abinfo.fragsize;
			abinfo.fragments = abinfo.bytes / abinfo.fragsize;
			spin_unlock_irqrestore(&s->lock, flags);

			KDEBUG("sm501_ioctl: SNDCTL_DSP_GETOSPACE -> fragsize=%d,bytes=%d\n",
			       abinfo.fragsize, abinfo.bytes);
			if (copy_to_user((void*) arg, &abinfo, sizeof(abinfo)))
			    return(-EFAULT);
			return(0);

		    case SNDCTL_DSP_GETISPACE:
			if (!(file->f_mode & FMODE_READ))
			    return(-EINVAL);

			if (!s->recording.ready)
			{
			    ret = program_recording(s);
			    if (ret != 0)
				return(ret);
			}

			spin_lock_irqsave(&s->lock, flags);
			abinfo.fragsize = s->recording.threshold;
			count = s->recording.count;
			if (count < 0)
			    count = 0;
			abinfo.bytes = count;
			abinfo.fragstotal = s->recording.sw_buffer_size / abinfo.fragsize;
			abinfo.fragments = abinfo.bytes / abinfo.fragsize;
			spin_unlock_irqrestore(&s->lock, flags);

			KDEBUG("sm501_ioctl: SNDCTL_DSP_GETISPACE -> fragsize=%d,bytes=%d\n",
			       abinfo.fragsize, abinfo.bytes);
			if (copy_to_user((void*) arg, &abinfo, sizeof(abinfo)))
			    return(-EFAULT);
			return(0);

		    case SNDCTL_DSP_NONBLOCK:
			KDEBUG("sm501_ioctl: SNDCTL_DSP_NONBLOCK\n");
			file->f_flags |= O_NONBLOCK;
			return(0);

		    case SNDCTL_DSP_GETODELAY:
			if (!(file->f_mode & FMODE_WRITE))
			    return(-EINVAL);

			if (!s->playing.ready)
			{
			    ret = program_playing(s);
			    if (ret != 0)
				return(ret);
			}

			spin_lock_irqsave(&s->lock, flags);
			count = s->playing.count;
			spin_unlock_irqrestore(&s->lock, flags);

			KDEBUG("sm501_ioctl: SNDCTL_DSP_GETODELAY -> %d\n", count);
			return(put_user(count, (int*) arg));

		    case SNDCTL_DSP_GETIPTR:
			if (!(file->f_mode & FMODE_READ))
			    return(-EINVAL);

			if (!s->recording.ready)
			{
			    ret = program_recording(s);
			    if (ret != 0)
				return(ret);
			}

			spin_lock_irqsave(&s->lock, flags);
			cinfo.bytes = s->recording.total_bytes;
			count = s->recording.count;
			if (count < 0)
			    count = 0;
			cinfo.blocks = count / s->recording.threshold;
			cinfo.ptr = s->recording.hw_ptr;
			spin_unlock_irqrestore(&s->lock, flags);

			KDEBUG("sm501_ioctl: SNDCTL_DSP_GETIPTR -> bytes=%d,block=%d,ptr=%u\n",
			       cinfo.bytes, cinfo.blocks, cinfo.ptr);
			if (copy_to_user((void*) arg, &cinfo, sizeof(cinfo)))
			    return(-EFAULT);
			return(0);

		    case SNDCTL_DSP_GETOPTR:
			if (!(file->f_mode & FMODE_WRITE))
			    return(-EINVAL);

			if (!s->playing.ready)
			{
			    ret = program_playing(s);
			    if (ret != 0)
				return(ret);
			}

			spin_lock_irqsave(&s->lock, flags);
			cinfo.bytes = s->playing.total_bytes;
			count = s->playing.count;
			if (count < 0)
			    count = 0;
			cinfo.blocks = count / s->playing.threshold;
			cinfo.ptr = s->playing.hw_ptr;
			spin_unlock_irqrestore(&s->lock, flags);

			KDEBUG("sm501_ioctl: SNDCTL_DSP_GETOPTR -> bytes=%d,block=%d,ptr=%u\n",
			       cinfo.bytes, cinfo.blocks, cinfo.ptr);
			if (copy_to_user((void*) arg, &cinfo, sizeof(cinfo)))
			    return(-EFAULT);
			return(0);

		    case SNDCTL_DSP_GETBLKSIZE:
			val = 0;

			if (file->f_mode & FMODE_WRITE)
			{
			    ret = program_playing(s);
			    if (ret != 0)
				return(ret);

			    val = s->playing.threshold;
			}

			if (file->f_mode & FMODE_READ)
			{
			    ret = program_recording(s);
			    if (ret != 0)
				return(ret);

			    val = s->recording.threshold;
			}

			KDEBUG("sm501_ioctl: SNDCTL_DSP_GETBLKSIZE -> %d\n", val);
			return(put_user(val, (int*) arg));

		    case SNDCTL_DSP_SETFRAGMENT:
//			return 0;				
		    case SNDCTL_DSP_SUBDIVIDE:
		    case SNDCTL_DSP_SETSYNCRO:
			return(-EINVAL);

		    case SOUND_PCM_READ_RATE:
			return(put_user((file->f_mode & FMODE_READ) ? s->recording.rate :
					s->playing.rate, (int*) arg));

		    case SOUND_PCM_READ_CHANNELS:
			return(put_user((file->f_mode & FMODE_READ) ? s->recording.channels :
					s->playing.channels, (int*) arg));

		    case SOUND_PCM_READ_BITS:
			return(put_user((file->f_mode & FMODE_READ) ? s->recording.bits :
					s->playing.bits, (int*) arg));

		    case SOUND_PCM_WRITE_FILTER:
		    case SOUND_PCM_READ_FILTER:
			return(-EINVAL);
		    }

		    return(s->codec->mixer_ioctl(s->codec, cmd, arg));
		}

static int sm501_oss_open(struct inode* inode, struct file* file)
		{
		    int minor = MINOR(inode->i_rdev);
		    //int minor = iminor(inode);
		    DECLARE_WAITQUEUE(wait, current);
		    unsigned long flags;
		    sm501Ptr s = host_data;
		    unsigned int value;

		    VALIDATE_STATE(s);

		    if (((s->dev_audio ^ minor) & ~0xF))
			return(-ENODEV);

		    file->private_data = s;

		    down(&s->open_sem);
		    while (s->open_mode & file->f_mode)
		    {
			KDEBUG("device already in use, waiting...\n");

			if (file->f_flags & O_NONBLOCK)
			{
			    up(&s->open_sem);
			    return(-EBUSY);
			}
			add_wait_queue(&s->open_wait, &wait);
			__set_current_state(TASK_INTERRUPTIBLE);
			up(&s->open_sem);
			schedule();
			remove_wait_queue(&s->open_wait, &wait);
			set_current_state(TASK_RUNNING);
			if (signal_pending(current))
			    return(-ERESTARTSYS);
			down(&s->open_sem);
		    }
		    if (file->f_mode & FMODE_READ)
		    {
		    //AC97_RECMUX_MIC
//		    	value = read_codec_wrapper(s->codec, AC97_AUX_VOL );    //06h
//			KDEBUG("sm501 aux vol: %d \n", value);
//			write_codec_wrapper(s->codec, AC97_AUX_VOL , 0x8000 );

//		    	value = read_codec_wrapper(s->codec, AC97_LINEIN_VOL );    //06h
//			KDEBUG("sm501 linein vol: %d \n", value);
//			write_codec_wrapper(s->codec, AC97_LINEIN_VOL , 0x8000 );

//		    	value = read_codec_wrapper(s->codec, AC97_MASTER_VOL_MONO );    //06h
//			KDEBUG("sm501 mono vol: %d \n", value);
//			write_codec_wrapper(s->codec,AC97_MASTER_VOL_MONO , 0x0020 );

		//	value = read_codec_wrapper(s->codec, AC97_PCBEEP_VOL );    //0ah
		//	KDEBUG("sm501 pc beep: %d \n", value);
		//	write_codec_wrapper(s->codec, AC97_PCBEEP_VOL, value | 0x8000 );

	// Write the stereo analog lines volume.
		//	write_codec_wrapper(s->codec, AC97_LINEIN_VOL, 0x0808);

			// Set up the record mux for line input.
		//	write_codec_wrapper(s->codec, AC97_RECORD_SELECT, AC97_RECMUX_MIC);

			// Enable variable rate control.
		//	write_codec_wrapper(s->codec, AC97_HEADPHONE_VOL, 0x3F3F);


		//	value = read_codec_wrapper(s->codec, AC97_RECORD_GAIN_MIC);    //1eh
		//	KDEBUG("sm501 pcm mic adc rate: %d \n", value);
		//	value = 0x0808;
		//	write_codec_wrapper(s->codec, AC97_RECORD_GAIN_MIC, value);

//		   	value = read_codec_wrapper(s->codec, AC97_PCM_LR_ADC_RATE);    //32h
//			KDEBUG("sm501 pcm L/R adc rate: %d \n", value);
//			value = 44100;
//			write_codec_wrapper(s->codec, AC97_PCM_LR_ADC_RATE, value);

//		   	value = read_codec_wrapper(s->codec, AC97_PCM_MIC_ADC_RATE);    //34h
//			KDEBUG("sm501 pcm mic adc rate: %d \n", value);
//			value = 44100;
//			write_codec_wrapper(s->codec, AC97_PCM_MIC_ADC_RATE, value);

		    	value = read_codec_wrapper(s->codec, AC97_RECORD_SELECT  );          // 1eh
			KDEBUG("sm501 record select: %d \n", value);
			write_codec_wrapper(s->codec, AC97_RECORD_SELECT , AC97_RECMUX_MIC );

			// Set up a default record gain.
   		       write_codec_wrapper(s->codec, AC97_RECORD_GAIN, 0x0);

		    	value = read_codec_wrapper(s->codec, AC97_GENERAL_PURPOSE);    //20h
			KDEBUG("sm501 general purpose: %d \n", value);
			write_codec_wrapper(s->codec, AC97_GENERAL_PURPOSE, value&~0x0100);

		   	value = read_codec_wrapper(s->codec, AC97_EXTENDED_STATUS);        // 2ah
			KDEBUG("sm501 extened status: %d \n", value);
			write_codec_wrapper(s->codec, AC97_EXTENDED_STATUS,
			                 value | AC97_EXTSTAT_VRA | AC97_EXTSTAT_VRM);

			s->recording.enabled = 1;
			set_recording_rate(s, 48000);//11025);                                  //32h

		    }
		    if (file->f_mode & FMODE_WRITE)
		    {
			KDEBUG("sm501 open play: \n");
			//hxp add
			s->playing.enabled = 1;
			set_playing_rate(s, 48000);
		    }

		    spin_lock_irqsave(&s->lock, flags);
		    if (file->f_mode & FMODE_READ)
		    {
			KDEBUG("sm501 open record: 1\n");
			//hxp add
			s->recording.bits = ((minor & 0xF) == SND_DEV_DSP16) ? 16 : 8;
			s->recording.channels = 1;
		#ifdef CONFIG_SOUND_SM501_DEBUG
			s->data_index = 0;
		#endif			
		    }
			
		    if (file->f_mode & FMODE_WRITE)
		    {
			KDEBUG("sm501 open play: 1 \n");
			//hxp add
			s->playing.bits = ((minor & 0xF) == SND_DEV_DSP16) ? 16 : 8;
			s->playing.channels = 1;
		#ifdef CONFIG_SOUND_SM501_DEBUG
			s->data_index = 0;
		#endif
		    }
		    spin_unlock_irqrestore(&s->lock, flags);

		    s->open_mode |= file->f_mode & (FMODE_READ | FMODE_WRITE);
		    up(&s->open_sem);
		    init_MUTEX(&s->sem);
			KDEBUG("sm501 open success!\n");
			//hxp add
		    return(nonseekable_open(inode, file));
		    //return(0);
		}

		static int sm501_oss_release(struct inode* inode, struct file* file)
		{
		    sm501Ptr s = (sm501Ptr) file->private_data;
		    VALIDATE_STATE(s);

		    KDEBUG("release: file=%p\n", file);

		    lock_kernel();
		    if (file->f_mode & FMODE_WRITE)
			drain_playing(s, file->f_flags & O_NONBLOCK);

		    down(&s->open_sem);
		    if (file->f_mode & FMODE_WRITE)
		    {
			stop_playing(s);
			dealloc_buffer(s, &s->playing);
		    KDEBUG("release play\n");
		    }
		    if (file->f_mode & FMODE_READ)
		    {
			stop_recording(s);
			dealloc_buffer(s, &s->recording);
		    KDEBUG("release record\n");
		    }
		    s->open_mode &= ~(file->f_mode & (FMODE_READ | FMODE_WRITE));
		    up(&s->open_sem);

		    wake_up(&s->open_wait);
		    unlock_kernel();
		    return(0);
		}

		static struct file_operations sm501_audio_fops =
		{
		    .owner=      THIS_MODULE,
		    .llseek=     no_llseek,
		    .read=       sm501_oss_read,
		    .write=      sm501_oss_write,
		    .poll=       sm501_oss_poll,
		    .ioctl=      sm501_oss_ioctl,
		    .open=       sm501_oss_open,
		    .release=    sm501_oss_release,
		};

		/* --------------------------------------------------------------------- */

		#ifdef CONFIG_SOUND_SM501_DEBUG
		#define DUMP(msg, args...) \
		do { \
		    len += sprintf(buf + len, msg, ##args); \
		    if (len >= limit) \
		    { \
			*start = buf; \
			return(len); \
		    } \
		} while (0)

		static int proc_sm501_dump(char* buf, char** start, off_t fpos, int length,
					   int* eof, void* data)
		{
		    sm501Ptr s;
		    int i, len = 0;
		    int limit = length - 80;

		    s = data;
		    if (s == NULL)
		    {
			*eof = 1;
			return(0);
		    }

		    /* Print out header. */
		    DUMP("Silicon Motion SM501 Debug Dump-o-matic\n");
		    DUMP("=======================================\n");

		    /* Print out PCI config space. */
/*
		    if (s->dev != NULL)
		    {
			DUMP("  PCI Configuration space:\n");
			for (i = 0; i < 64; i += 4)
			{
			    unsigned int value;
			    pci_read_config_dword(s->dev, i, &value);
			    if ((i & 15) == 0)
				DUMP("    %02x:", i);
			    DUMP(" %08x", value);
			    if ((i & 15) == 12)
				DUMP("\n");
			}
		    }
*/
		    /* Print out some registers. */
		    DUMP("  Register dump:\n");
		    DUMP("    SYSTEM_CONTROL:        %08x\n", SmRead32(SYSTEM_CTRL));
		    DUMP("    MISCELLANEOUS_CONTROL: %08x\n", SmRead32(MISC_CTRL));
		    DUMP("    GPIO_CONTROL_LOW:      %08x\n", SmRead32(GPIO_CONTROL_LOW));
		    DUMP("    INTERRUPT_STATUS:      %08x\n", SmRead32(INTERRUPT_STATUS));
		    DUMP("    INTERRUPT_MASK:        %08x\n", SmRead32(INTERRUPT_MASK));
		    DUMP("    CURRENT_GATE:          %08x\n", SmRead32(CURRENT_GATE));
		    DUMP("    CURRENT_CLOCK:         %08x\n", SmRead32(CURRENT_POWER_CLOCK));
		    DUMP("    MISCELLANEOUS_TIMING:  %08x\n", SmRead32(MISCELLANEOUS_TIMING));
		    DUMP("    U8051_RESET:           %08x\n", SmRead32(U8051_RESET));
		    DUMP("    U8051_MODE_SELECT:     %08x\n", SmRead32(U8051_MODE_SELECT));

		    /* Print out some SRAM. */
/*
		    DUMP("  SRAM dump:\n");
		    for (i = 0; i < 64; i++)
		    {
			if ((i & 7) == 0)
			    DUMP("    %05x:", SRAM(ac97_regs[i]));
			if ((i & 7) == 4)
			    DUMP(" ");
			DUMP(" %04x", SmRead16(SRAM(ac97_regs[i])));
			if ((i & 7) == 7)
			    DUMP("\n");
		    }

		    for (i = SRAM(command_byte); i <= SRAM(init_count); i++)
		    {
			if ((i & 15) == 0)
			    DUMP("    %05x:", i);
			if ((i & 15) == 8)
			    DUMP(" ");
			DUMP(" %02x", SmRead8(i));
			if ((i & 15) == 15)
			    DUMP("\n");
		    }
			DUMP("\n");
*/
		    /* Print out the codec state. */
/*			
		    DUMP("  AC97 CODEC state:\n");
		    for (i = 0; i < 0x80; i += 2)
		    {
			if ((i & 15) == 0)
			    DUMP("    %02x:", i);
			if ((i & 15) == 8)
			    DUMP(" ");
			DUMP(" %04x", read_codec_wrapper(s->codec, i));
			if ((i & 15) == 14)
			    DUMP("\n");
		    }
*/
		    *eof = 1;
		    return(len);

		}
		static int proc_AC97_dump(char* buf, char** start, off_t fpos, int length,
					   int* eof, void* data)
		{
		    sm501Ptr s;
		    int i, len = 0;
		    int limit = length - 80;

		    s = data;
		    if (s == NULL)
		    {
			*eof = 1;
			return(0);
		    }

		    /* Print out header. */
		
		    DUMP("  AC97 CODEC state:\n");
		    for (i = 0; i < 0x70; i += 2)
		    {
			if ((i & 15) == 0)
			    DUMP("    %02x:", i);
			if ((i & 15) == 8)
			    DUMP(" ");
			DUMP(" %04x", read_codec_wrapper(s->codec, i));
			if ((i & 15) == 14)
			    DUMP("\n");
		    }

		    *eof = 1;
		    return(len);
			
		}
		static int proc_sm501_sample(char* buf, char** start, off_t fpos, int length,
					     int* eof, void* data)
		{
		    sm501Ptr s;
		    int len = 0;
		    int limit = length - 80;
		    static int i, j;

		    s = data;
		    if (s == NULL)
		    {
			*eof = 1;
			return(0);
		    }

		    /* Print out header. */
		    if (fpos == 0)
		    {
			DUMP("Silicon Motion SM501 Sample Debug Dump-o-matic\n");
			DUMP("==============================================\n");
			i = j = 0;
		    }

		    if (j == 0)
		    {
			/* Print out the sample buffer. */
			if (i == 0)
			    DUMP("  Sample buffer: %d samples.\n", s->sample_index);

			for (i &= ~3; i < s->sample_index; i++)
			{
			    if ((i & 3) == 0) DUMP("\r%9d:", i);
			    DUMP(" %08x", s->samples[i]);
			    if ((i & 3) == 3) DUMP("\n");
			}

			/* Print out the data buffer. */
			i = 0;
			j = 1;
			DUMP("\n  Data buffer: %d bytes.\n", s->data_index);
		    }

		    for (i &= ~15; i < s->data_index; i++)
		    {
			if ((i & 15) == 0) DUMP("\r%9d:", i);
			if ((i & 15) == 8) DUMP(" ");
			DUMP(" %02x", s->data[i]);
			if ((i & 15) == 15) DUMP("\n");
		    }

		    *eof = 1;
		    return(len);
		}
		#endif

		static struct initvol
		{
		    int mixch;
		    int vol;
		}
		initvol[] __initdata =
		{
		    { SOUND_MIXER_WRITE_LINE, 0x4040 },
		    { SOUND_MIXER_WRITE_CD, 0x4040 },
		    { MIXER_WRITE(SOUND_MIXER_VIDEO), 0x4040 },
		    { SOUND_MIXER_WRITE_LINE1, 0x4040 },
		    { SOUND_MIXER_WRITE_PCM, 0x4040 },
		    { SOUND_MIXER_WRITE_VOLUME, 0x4040 },
		    { MIXER_WRITE(SOUND_MIXER_PHONEOUT), 0x4040 },
		    { SOUND_MIXER_WRITE_OGAIN, 0x4040 },
		    { MIXER_WRITE(SOUND_MIXER_PHONEIN), 0x4040 },
		    { SOUND_MIXER_WRITE_SPEAKER, 0x4040 },
		    { SOUND_MIXER_WRITE_MIC, 0x4040 },
		    { SOUND_MIXER_WRITE_RECLEV, 0x4040 },
		    { SOUND_MIXER_WRITE_IGAIN, 0x4040 }
		};

		static unsigned int __devinit ascii_hex(const char* text, int* index, int count)
		{
		    /* Initialize result. */
		    unsigned int result = 0;

		    /* Convert nibbles. */
		    while (count-- > 0)
		    {
			/* Get Ascii nibble. */
			char c = text[*index];

			/* Convert from Ascii into hex. */
			if (c >= '0' && c <= '9')
			    c -= '0';
			else if (c >= 'A' && c <= 'F')
			    c -= 'A' - 10;
			else if (c >= 'a' && c <= 'f')
			    c -= 'a' - 10;
			else
			    /* Invalid digits, return 0. */
			    return 0;

			/* Add nibble to result. */
			result = (result << 4) | c;

			/* Increment string index. */
			(*index)++;
		    }

		    /* Return result. */
		    return result;
		}

static int __devinit load_firmware(sm501Ptr s, const char* firmware)
{
	unsigned int value;
	int i, errors, retry=1;

	/* Put 8051 in reset state. */
	value = SET_FIELD(0, U8051_RESET_CONTROL, RESET);
	SmWrite32( U8051_RESET, value);

	/* Retry loop for loading firmware. */
	for (retry = 1; retry <= 1; retry++)
	{
		KDEBUG("loading firmware: retry %d\n", retry);

		/* Reset error count. */
		errors = 0;

		/* !!BUGBUG!! Settle down 8051 or SRAM, seems to be required. */
		mdelay(250);

		/* Zero SRAM. */
		SmMemset(U8051_SRAM, 0, U8051_SRAM_SIZE);

		/* Walk entire firmware. */
		for (i = 0; i < strlen(firmware);)
		{
			unsigned int bytes, address, finish;

			/* Skip colon (start of line). */
			if (firmware[i] != ':')
			{
				KERROR("invalid firmware character '%c'\n",
				       firmware[i]);
				break;
			}
			i++;

			/* Get number of bytes (first byte). */
			bytes = ascii_hex(firmware, &i, 2);

			/* Get address (second and third byte). */
			address = ascii_hex(firmware, &i, 4);

			/* Get finish flag (fourth byte). */
			finish = ascii_hex(firmware, &i, 2);

			/* Loop through all bytes. */
			while (bytes-- > 0)
			{
				/* Get byte. */
				unsigned int value = ascii_hex(firmware,
						&i, 2);

				/* Write byte into SRAM address. */
				SmWrite8( U8051_SRAM + address,
						value);
				if (SmRead8( U8051_SRAM + address)
								!= value)
				{
					if (errors++ < 10)
					{
						KWARNING("%04x: mismatch: "
								"code(%02x) "
								"sram(%02x)\n",
						address, value,
						SmRead8( U8051_SRAM + address));
					}
				}

				/* Increment address. */
				address++;
			}

			/* Skip checksum. */
			i += 2;

			/* Finish when end of data. */
			if (finish != 0)
				break;
		}

		if (errors == 0)
			break;
	}

	if (errors > 0)
		return(-EFAULT);

	/* Enable 8051. */
	value = SET_FIELD(0, U8051_RESET_CONTROL, ENABLE);
	SmWrite32( U8051_RESET, value);

	/* Wait until the init is complete. During its initialization the
	firmware will increment the "init counter" twice, so that when the
	counter becomes 2, we know that the initialization is complete. */
	
	i = 5000;
	while (SmRead8( SRAM(init_count)) != 2)
	{
		if (i-- <= 0)
		{
			KERROR("firmware timed out during reset\n");
			return(-ETIME);
		}
		mdelay(1);
	}

	KDEBUG("firmware loaded and running\n");
	return 0;
}

static int sm501_oss_remove(struct platform_device * pdev)
		{
		    unsigned int value;
		    sm501Ptr s = NULL;

		    s = platform_get_drvdata(pdev);
		    platform_set_drvdata(pdev, s->dev);
		
		    KDEBUG("cleanup: s=%p\n", s);
		    if (s == NULL)
			return;

		#ifdef CONFIG_SOUND_SM501_DEBUG
		    if (s->ps != NULL)
			remove_proc_entry("driver/sm501_audio", NULL);
		    if (s->ac97ps != NULL)
			remove_proc_entry("driver/sm501_ac97", NULL);			
		#endif

		    if (s->irq)
		    {
			/* Remove interrupt service handler. */
		       synchronize_irq(s->irq);
			free_irq(s->irq, s);
		    }

		    if (s->dev_audio != 0)
		    {
			/* Unregister device. */
			unregister_sound_dsp(s->dev_audio);
		    }

		    if (s->codec != NULL)
		    {
			/* Unregister mixer device. */
			if (s->codec->dev_mixer != -1)
			    unregister_sound_mixer(s->codec->dev_mixer);

			/* Release codec. */
			ac97_release_codec(s->codec);
		    }

		    if (s->io != NULL)
		    {
			/* Disable 8051 interrupt mask. */
			value = SmRead32(INTERRUPT_MASK);
			value = SET_FIELD(value, INTERRUPT_MASK_8051, DISABLE);
			SmWrite32(INTERRUPT_MASK, value);

			/* Put 8051 in reset state. */
			value = SmRead32(U8051_RESET);
			value = SET_FIELD(value, U8051_RESET_CONTROL, RESET);
			SmWrite32(U8051_RESET, value);

			/* Disable SRAM. */
			value = SmRead32(U8051_MODE_SELECT);
			value = SET_FIELD(value, U8051_MODE_SELECT_SRAM, DISABLED);
			SmWrite32(U8051_MODE_SELECT, value);

			/* Reset GPIO lines. */
			value = SmRead32(GPIO_CONTROL_LOW);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_28, GPIO);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_27, GPIO);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_26, GPIO);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_25, GPIO);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_24, GPIO);
			SmWrite32(GPIO_CONTROL_LOW, value);

			/* Disable 8051 and AC97. */
			value = SmRead32(CURRENT_GATE);
			value = SET_FIELD(value, CURRENT_GATE_AC97_I2S, DISABLE);
			value = SET_FIELD(value, CURRENT_GATE_8051, DISABLE);
			sm501_set_gate( value);

		    }

		#ifdef CONFIG_SOUND_SM501_DEBUG
		    if (s->sample_ps != NULL)
			remove_proc_entry("driver/sm501sample", NULL);
		    if (s->samples != NULL)
			kfree(s->samples);
		    if (s->data != NULL)
			kfree(s->data);
		#endif

		    /* Free state structure. */
		    kfree(s);
		    KDEBUG("cleanup: done\n");
}


static int sm501_oss_probe(struct platform_device *pdev)
{
		    mm_segment_t fs;
		    unsigned int value, clock;
		    int res = -1, i=0,ret=0;
		    struct device  *pcidev = NULL;
		    sm501Ptr s = NULL;

		    KDEBUG("SM501 audio -- probe:\n");

			s = kmalloc(sizeof(sm501Rec),GFP_KERNEL);

	   	      if (s == NULL)
	 	      {
			  KERROR("out of memory\n");
			  return(-ENOMEM);
			}
	             memset(s, 0, sizeof(sm501Rec));

			s->dev = pcidev = platform_get_drvdata(pdev);
			platform_set_drvdata(pdev, s);

		    /* Store state structure pointer. */
			host_data = s;

			/* Allocate codec. */
			s->codec = ac97_alloc_codec();
			if (s->codec == NULL)
			    goto failed;
			KDEBUG("codec mixer: %d\n", s->codec->dev_mixer);
			s->codec->dev_mixer = -1;

			/* Initialize some queues and stuff. */
			init_waitqueue_head(&s->playing.wait);
			init_waitqueue_head(&s->recording.wait);
			init_waitqueue_head(&s->open_wait);
			init_MUTEX(&s->open_sem);
			spin_lock_init(&s->lock);

			s->irq = platform_get_irq(pdev, 0);
			if (s->irq < 0) {
				/* we currently do not use the IRQ */
				KERROR( "no irq for device\n");
				return s->irq;
			}

			/* Initial audio chip interrupt     */
			ret = request_irq(s->irq, &sm501_interrupt, IRQF_SHARED,"sm501_audio", s);
			if (ret < 0) {
				KERROR( "inital sm501_audio: unable to get irq %d\n",s->irq);
				return ret;
			}
		
			/* Initialize codec stuff. */
			s->codec->private_data = s;
			s->codec->id = 0;
			s->codec->codec_read = read_codec_wrapper;
			s->codec->codec_write = write_codec_wrapper;
			s->codec->codec_wait = wait_codec;

			/* Register device. */
			res = s->dev_audio = register_sound_dsp(&sm501_audio_fops, -1);
			if (res < 0)
				goto failed;
				res = s->codec->dev_mixer = register_sound_mixer(&sm501_mixer_fops, -1);
			if (res < 0)
				goto failed;
				KDEBUG("registered mixer %d\n", s->codec->dev_mixer);

		#ifdef CONFIG_SOUND_SM501_DEBUG
			/* Intialize the debug proc device. */
			s->ps = create_proc_read_entry("driver/sm501_audio", 0, NULL, proc_sm501_dump, s);

			s->ac97ps = create_proc_read_entry("driver/sm501_ac97", 0, NULL, proc_AC97_dump, s);

			s->sample_size = 32768;
			s->samples = kmalloc(s->sample_size * sizeof(int), GFP_KERNEL);
			if (s->samples == NULL)
			{
			    KWARNING("unable to allocate %d-entry sample buffer\n",
				     s->sample_size);
			    s->sample_size = 0;
			}
			else
			{
			    s->data_size = 16384;
			    s->data = kmalloc(s->data_size, GFP_KERNEL);
			    if (s->data == NULL)
			    {
				KWARNING("unable to allocate %d-byte data buffer\n",	 s->data_size);
				s->data_size = 0;
			    }

			    s->sample_ps = create_proc_read_entry("driver/sm501sample", 0, NULL,
								  proc_sm501_sample, s);
			}
		#endif


			value = SmRead32(GPIO_CONTROL_LOW);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_28, AC97_RX_I2S);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_27, AC97_TX_I2S);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_26, AC97_CLOCK_I2S);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_25, AC97_SYNC_I2S);
			value = SET_FIELD(value, GPIO_CONTROL_LOW_24, AC97_RESET);
			SmWrite32(GPIO_CONTROL_LOW, value);

	
			// Set arbitartion control.
			value = SmRead32(ARBITRATION_CONTROL);
			value = SET_FIELD(value, ARBITRATION_CONTROL_DMA, PRIORITY_1);
			SmWrite32(ARBITRATION_CONTROL, value);
	
			value = SmRead32(INTERRUPT_MASK);
			value = SET_FIELD(value, INTERRUPT_MASK_8051, ENABLE);
			value = SET_FIELD(value, INTERRUPT_MASK_DMA, ENABLE);
			SmWrite32(INTERRUPT_MASK, value);

			/* Initialize chip. */
			value = SmRead32(CURRENT_GATE);
			value = SET_FIELD(value, CURRENT_GATE_AC97_I2S, ENABLE);
			value = SET_FIELD(value, CURRENT_GATE_8051, ENABLE);
			value = SET_FIELD(value, CURRENT_GATE_HOST_COMMAND_LIST_DMA, ENABLE);
			value = SET_FIELD(value, CURRENT_GATE_GPIO_PWM_I2C, ENABLE);
			sm501_set_gate(value);
			
			value = SmRead32(U8051_MODE_SELECT);
			value = SET_FIELD(value, U8051_MODE_SELECT_SRAM, ENABLED);
			value = SET_FIELD(value, U8051_MODE_SELECT_TEST_MODE, NORMAL);
			value = SET_FIELD(value, U8051_MODE_SELECT_CODEC_SELECT, AC97);
			value = SET_FIELD(value, U8051_MODE_SELECT_DIVIDER, 2);
			SmWrite32(U8051_MODE_SELECT, value);

			/* Initialize firmware. */
			res = load_firmware(s, FIRMWARE);
			if (res < 0)
			    goto failed;
			
			/* Codec init. */
			if (!ac97_probe_codec(s->codec))
			{
			    KERROR("ac97_probe_codec failed\n");
			    res = -ENODEV;
			    goto failed;
			}

//			InitCodec(s);
			/* Determine variable bit rate support. */
	//		value = AC97_EA_VRM | AC97_EA_VRA;
	//		write_codec_wrapper(s->codec, AC97_EXTENDED_STATUS, value);
			value = read_codec_wrapper(s->codec, AC97_EXTENDED_ID);

			s->playing.vbr = (value & AC97_EA_VRA) ? 1 : 0;
			s->recording.vbr = (value & AC97_EA_VRM) ? 1 : 0;

			/* Enable variable bit rate. */
			value &= AC97_EA_VRA | AC97_EA_VRM;
			write_codec_wrapper(s->codec, AC97_EXTENDED_STATUS, value);
			//KDEBUG("vbr for playing=%d, recording=%d\n", s->playing.vbr,
			KDEBUG("vbr for playing=%d, recording=%d\n", s->playing.vbr,
			       s->recording.vbr);
			
		    	value = read_codec_wrapper(s->codec, AC97_MIC_VOL);            //0eh
			KDEBUG("sm501 mic vol: %x \n", value);
			write_codec_wrapper(s->codec, AC97_MIC_VOL, value|0x8000&~0x40);

		       SmWrite8(SRAM(buffer_status), 0x0);
			   
			/* Set default values. */
			fs = get_fs();
			set_fs(KERNEL_DS);
			value = SOUND_MASK_LINE;
			s->codec->mixer_ioctl(s->codec, SOUND_MIXER_WRITE_RECSRC,
					      (unsigned long) &value);
			for (i = 0; i < sizeof(initvol) / sizeof(initvol[0]); i++)
			{
			    value = initvol[i].vol;
			    s->codec->mixer_ioctl(s->codec, initvol[i].mixch,
						  (unsigned long) &value);
			}
			set_fs(fs);
			return(0);

		failed:
		    /* Uninitialize anything we have done so far. */
		    sm501_oss_remove(pdev);
		    KDEBUG("probe: error=%d\n", res);
		    return(res);
}


static struct platform_driver sm501_ac97_driver = {
			.remove		= sm501_oss_remove,
			.probe		= sm501_oss_probe,
			.driver		= {
			.name	= "sm501-audio",
				.owner	= THIS_MODULE,
			},
};

static int __devinit sm501_oss_init(void)
{
    return platform_driver_register(&sm501_ac97_driver);
}

static void __exit sm501_oss_cleanup(void)
{
	platform_driver_unregister(&sm501_ac97_driver);
}


module_init(sm501_oss_init);
module_exit(sm501_oss_cleanup);

		
MODULE_AUTHOR("Frido Garritsen, Boyod Yang");
MODULE_DESCRIPTION("SM501 OSS Driver");
MODULE_LICENSE("Dual BSD/GPL");

