/*
 * AC97 support for SM50X chipsets
 *
 * Based on Silicon Motion drivers
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/ac97_codec.h>
#include <sound/initval.h>
#include <sound/soc.h>

#include <asm/irq.h>
#include <linux/mutex.h>

#include <linux/sm501.h>
#include <linux/sm501_regs.h>

#include "sm501-ac97.h"
#include "sm501-pcm.h"
#include "sm501-u8051.h"
#include "audfw.hex"

/*
 * Register address in the firmware
 */
#define SET_TX_SLOT_0                    0
#define SET_TX_SLOT_1                    1
#define SET_TX_SLOT_2                    2
#define SET_PLAYBACK_BUFFER_ADDRESS      3
#define SET_PLAYBACK_BUFFER_SIZE         4
#define SET_PLAYBACK_BUFFER_READY        5
#define GET_BUFFER_STATUS                6
#define START_STOP_AUDIO_PLAYBACK        7
#define GET_RX_SLOT0                     8
#define GET_RX_SLOT1                     9
#define GET_RX_SLOT2                    10
#define SET_CAPTURE_BUFFER_ADDRESS      11
#define SET_CAPTURE_BUFFER_SIZE         12
#define SET_CAPTIRE_BUFFER_EMPTY        13
#define START_STOP_AUDIO_CAPTURE        14
#define SET_GET_AC97_REGISTER           15

#define DRV_NAME			"sm501-audio"

static int debug        = 0;
#define dprintk(msg...) if (debug) { printk(KERN_DEBUG DRV_NAME ": " msg); }
static struct sm501_audio *audio_dev;

static void sm501_write_codec(struct sm501_audio *s, u8 addr, u16 data)
{
	unsigned int read_back;

	//dprintk("%s: addr=%02x data=%04x\n", __func__, addr, data);

	/* Write Codec register. */
	if (!sm501_write_command(s, SET_GET_AC97_REGISTER, (unsigned int) addr << 12,
	     (unsigned int) data << 4) && (addr != AC97_RESET))
	{
		//if write failed 
		read_back = MAKEDWORD(MAKEWORD(SmRead8(SRAM(data_byte[4])),
					SmRead8(SRAM(data_byte[5]))),
					MAKEWORD(SmRead8(SRAM(data_byte[6])),
					SmRead8(SRAM(data_byte[7])))) >> 4;

		/*
		 * Not dev_err here because the firmware returns an error
		 * for R/O or not defined registers
		 */
		printk(s->dev, "%s failed: addr=%02x data=%04x read_back=%04x\n",
			__func__, addr, data, read_back);
	}
	else
	{	
		printk(s->dev, "%s ok: addr=%02x data=%04x\n",
		__func__, addr, data);
	}
}

#if 1
static u16 sm501_read_codec(struct sm501_audio * s,u8 addr)
{
	int failed;
	u32 data;

	//according to the implement of firmware,we can't tell if the read operate okay or not
	//read command always return a 1 value
	//but put the failed flag here for futhure nice upgrade
	failed = !sm501_write_command(s,SET_GET_AC97_REGISTER,(u32)(addr<<12)|(1<<19),0);	
	data = MAKEDWORD(MAKEWORD(SmRead8(SRAM(data_byte[4])),
			SmRead8(SRAM(data_byte[5]))),
			MAKEWORD(SmRead8(SRAM(data_byte[6])),
			SmRead8(SRAM(data_byte[7])))) >> 4;
	
	if(failed)
	{	
		//read codec register failed
		printk("%s: failed: addr=%02x data=%04x status=%02x\n", __func__, addr,data,SmRead8(SRAM(status_byte)));
	}
	return (u16)data;
}
#else
static u16 sm501_read_codec(struct sm501_audio *s, u8 addr)
{
	unsigned int data;
	int i;

	/* Read Codec register. */
	if (sm501_write_command(s, SET_GET_AC97_REGISTER,
	    (unsigned int) (addr << 12) | (1 << 19), 0)) {
		data = MAKEDWORD(MAKEWORD(SmRead8(SRAM(data_byte[4])),
				SmRead8(SRAM(data_byte[5]))),
				MAKEWORD(SmRead8(SRAM(data_byte[6])),
				SmRead8(SRAM(data_byte[7])))) >> 4;
	} else {
		/* Read register from SRAM in case link failes. */
		for (i = SM501_AC97_CODEC_TIMEOUT; i > 0; i--) {
			data = SmRead16(SRAM(ac97_regs[addr >> 1]));
			if (data != 0xFFFF)
				break;
			msleep(1);
		}

		dev_err(s->dev, "%s failed: addr=%02x sram=%04x\n", __func__, addr, data);
	}

	dprintk("%s: data=%04x\n", __func__, data);
	return((unsigned short) data);
}
#endif
/*
 * AC97 functions
 */
static unsigned short snd_sm501_ac97_read(struct snd_ac97 *ac97, unsigned short reg)
{
	unsigned int data;
	unsigned long value;
	unsigned long flags;
	unsigned long i;

	spin_lock_irqsave(&audio_dev->lock, flags);

	if (audio_dev->firmware_loaded) {
		data = sm501_read_codec(audio_dev, reg);
	} else {

		SmWrite32(AC97_TX_SLOT2, 0);

		value = (unsigned long) reg << 12;
		value = SET_FIELD(value, AC97_TX_SLOT1_READ_WRITE, READ);
		SmWrite32(AC97_TX_SLOT1, value);

		/* Enable Slot 0, 1, 2 for read */
		value = SmRead32(AC97_TX_SLOT0);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_FRAME_TAG,
				  INTERPRET);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_S1, DATA);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_S2, DATA);
		SmWrite32(AC97_TX_SLOT0, value);

		udelay(150);

		for (i = SM501_AC97_CODEC_TIMEOUT; i> 0; i--)
		{
			if (TEST_FIELD(SmRead32(AC97_RX_SLOT0),
			    AC97_RX_SLOT0_CODEC_READY_FLAG, READY))
			{
				if ((TEST_FIELD(SmRead32(AC97_RX_SLOT0),
				     AC97_RX_SLOT0_VALID_S1,
				     DATA)) &&
					 (TEST_FIELD(SmRead32(AC97_RX_SLOT0),
						     AC97_RX_SLOT0_VALID_S2, DATA)))
					break;
			}
			udelay(1);
		}
		if (i == 0)
			dev_err(audio_dev->dev, "%s: AC97 timeout\n", __func__);


		data = SmRead32(AC97_RX_SLOT2);
		data = GET_FIELD(data, AC97_RX_SLOT2_DATA);

		/* Reset Slot 1,2 */
		value = SmRead32(AC97_TX_SLOT0);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_S1, NO_DATA);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_S2, NO_DATA);
		SmWrite32(AC97_TX_SLOT0, value);
	}

	spin_unlock_irqrestore(&audio_dev->lock, flags);

	return((unsigned short) data);
}

static snd_sm501_ac97_wait_codec(struct snd_ac97 * ac97)
{
	int i;
	u16 data;
	unsigned long flags;

	spin_lock_irqsave(&audio_dev->lock, flags);
	for(i=0;i<1000;i++)
	{
		data = sm501_read_codec(audio_dev,AC97_POWERDOWN);
		if(data & 2)
			break;
	}
	spin_unlock_irqrestore(&audio_dev->lock, flags);
}


static void snd_sm501_ac97_write(struct snd_ac97 *ac97, unsigned short reg,
				 unsigned short val)
{
	unsigned long flags;
	unsigned long value;
	unsigned long i;

	spin_lock_irqsave(&audio_dev->lock, flags);

	if (audio_dev->firmware_loaded) {
		sm501_write_codec(audio_dev, reg, val);
	} else {
		value = (unsigned long) reg << 12;
		SmWrite32(AC97_TX_SLOT1, value);

		value = ((unsigned int) val << 4);
		SmWrite32(AC97_TX_SLOT2, value);

		/* Enable Slot 0, 1, 2 for write */
		value = SmRead32(AC97_TX_SLOT0);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_FRAME_TAG,
				  INTERPRET);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_S1, DATA);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_S2, DATA);
		SmWrite32(AC97_TX_SLOT0, value);
		for (i = SM501_AC97_CODEC_TIMEOUT; i> 0; i--)
		{
			if (TEST_FIELD(SmRead32(
			    AC97_RX_SLOT0),AC97_RX_SLOT0_CODEC_READY_FLAG,
			READY))
			{
				if ((TEST_FIELD(SmRead32(AC97_RX_SLOT0),
				     AC97_RX_SLOT0_VALID_S1,
				     NO_DATA)) &&
				(TEST_FIELD(SmRead32(AC97_RX_SLOT0),
						     AC97_RX_SLOT0_VALID_S2,
						     NO_DATA)))
					break;
			}
			udelay(1);
		}
		if (i == 0)
			dev_err(audio_dev->dev, "%s: AC97 timeout\n", __func__);

		/* Reset Slot 1,2 */
		value = SmRead32(AC97_TX_SLOT0);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_S1, NO_DATA);
		value = SET_FIELD(value, AC97_TX_SLOT0_VALID_S2, NO_DATA);
		SmWrite32(AC97_TX_SLOT0, value);
	}

	spin_unlock_irqrestore(&audio_dev->lock, flags);
}

static void snd_sm501_ac97_warm_reset(struct snd_ac97 *ac97)
{
#warning FILL ME
}

static void snd_sm501_ac97_cold_reset(struct snd_ac97 *ac97)
{
#warning FILL ME
}


struct snd_ac97_bus_ops soc_ac97_ops = {
	.read		= snd_sm501_ac97_read,
	.write		= snd_sm501_ac97_write,
	.warm_reset	= snd_sm501_ac97_warm_reset,
	.reset		= snd_sm501_ac97_cold_reset,
	.wait	= snd_sm501_ac97_wait_codec,
};
EXPORT_SYMBOL_GPL(soc_ac97_ops);

static struct sm501_pcm_dma_params sm501_ac97_pcm_stereo_out;
static struct sm501_pcm_dma_params sm501_ac97_pcm_stereo_in;

/* hw_params callback */
static int snd_sm501_ac97_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		cpu_dai->dma_data = &sm501_ac97_pcm_stereo_out;
	else
		cpu_dai->dma_data = &sm501_ac97_pcm_stereo_in;

	return 0;
}
static int snd_sm501_ac97_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	int reg;
	u16 vra;

	vra = sm501_read_codec(audio_dev, AC97_EXTENDED_STATUS);
	sm501_write_codec(audio_dev, AC97_EXTENDED_STATUS, vra | AC97_EA_VRA);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		reg = AC97_PCM_FRONT_DAC_RATE;
	 else
		reg = AC97_PCM_LR_ADC_RATE;

	 sm501_write_codec(audio_dev, reg, runtime->rate);
	 return 0;
}

static int snd_sm501_ac97_probe(struct platform_device *pdev)
{
	struct device   *dev = &pdev->dev;
	struct resource *res;
	int retry = 0;
	int ret = 0;
	int res_len;
	unsigned long value;

	audio_dev = kzalloc(sizeof(struct sm501_audio), GFP_KERNEL);
	if (!audio_dev) {
		dev_err(dev, "Allocation failure\n");
		ret = -ENOMEM;
		goto err;
	}
	audio_dev->dev = dev;
	//monk add
	audio_dev->hw_buffer_addr = SmIoAddress() + 0xC3000;
	
	sm501_ac97_pcm_stereo_out.s = audio_dev;
	sm501_ac97_pcm_stereo_in.s = audio_dev;

	/* Get irq number */
//audio_dev->irq = 37; //platform_get_irq(pdev, 0);
//mj chang
	audio_dev->irq = 36; //platform_get_irq(pdev, 0);
	if (!audio_dev->irq) {
		dev_err(dev, "no irq found\n");
		ret = -ENODEV;
		goto err_alloc;
	}

	/* Get regs address */
	res = (struct resource *)kzalloc(sizeof(struct resource), GFP_KERNEL);
	if (res == NULL) {
		dev_err(dev, "No memory resource found\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

restart:

//Reset AC97 codec

	/* Put 8051 in reset state. */
        value = SmRead32(U8051_RESET);
        value = SET_FIELD(value, U8051_RESET_CONTROL, RESET);
        SmWrite32(U8051_RESET, value);

        /* Reset GPIO lines. */
        value = SmRead32(GPIO_CONTROL_LOW);
        value = SET_FIELD(value, GPIO_CONTROL_LOW_28, GPIO);
        value = SET_FIELD(value, GPIO_CONTROL_LOW_27, GPIO);
        value = SET_FIELD(value, GPIO_CONTROL_LOW_26, GPIO);
        value = SET_FIELD(value, GPIO_CONTROL_LOW_25, GPIO);
        value = SET_FIELD(value, GPIO_CONTROL_LOW_24, GPIO);
        SmWrite32(GPIO_CONTROL_LOW, value);

        /*Force GPIO28/SDIN GPIO27/SDOUT GPIO26/BIT_CLK GPIO25/SYNC to low*/
        value = SmRead32(GPIO_DATA_DIRECTION_LOW);
        value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_28, OUTPUT);
        value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_27, OUTPUT);
        value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_26, OUTPUT);
        value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_25, OUTPUT);
        SmWrite32(GPIO_DATA_DIRECTION_LOW, value);


        value = SmRead32(GPIO_DATA_LOW);
        value = SET_FIELD(value, GPIO_DATA_LOW_28, LOW);
        value = SET_FIELD(value, GPIO_DATA_LOW_27, LOW);
        value = SET_FIELD(value, GPIO_DATA_LOW_26, LOW);
        value = SET_FIELD(value, GPIO_DATA_LOW_25, LOW);
        SmWrite32(GPIO_DATA_LOW, value);


        /*Force GPIO24/AC97 Reset to low*/
        value = SmRead32(GPIO_DATA_DIRECTION_LOW);
        value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_24, OUTPUT);
        SmWrite32(GPIO_DATA_DIRECTION_LOW, value);


        value = SmRead32(GPIO_DATA_LOW);
        value = SET_FIELD(value, GPIO_DATA_LOW_24, LOW);
        SmWrite32(GPIO_DATA_LOW, value);
        msleep(10);

        /*Force GPIO24/AC97 Reset to high*/
        value = SmRead32(GPIO_DATA_DIRECTION_LOW);
        value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_24, OUTPUT);
        SmWrite32(GPIO_DATA_DIRECTION_LOW, value);

    
        value = SmRead32(GPIO_DATA_LOW);
        value = SET_FIELD(value, GPIO_DATA_LOW_24, HIGH);
        SmWrite32(GPIO_DATA_LOW, value);

////////////////////
	value = SmRead32(GPIO_CONTROL_LOW);
	value = SET_FIELD(value, GPIO_CONTROL_LOW_28, AC97_RX_I2S);
	value = SET_FIELD(value, GPIO_CONTROL_LOW_27, AC97_TX_I2S);
	value = SET_FIELD(value, GPIO_CONTROL_LOW_26, AC97_CLOCK_I2S);
	value = SET_FIELD(value, GPIO_CONTROL_LOW_25, AC97_SYNC_I2S);
	value = SET_FIELD(value, GPIO_CONTROL_LOW_24, AC97_RESET);
	SmWrite32(GPIO_CONTROL_LOW, value);

	/* Set arbitartion control */
	value = SmRead32(ARBITRATION_CONTROL);
	value = SET_FIELD(value, ARBITRATION_CONTROL_DMA, PRIORITY_1);
	SmWrite32(ARBITRATION_CONTROL, value);

	/* Enable interrupts */
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


	value = SmRead32(AC97_CONTROL_STATUS);
	value = SET_FIELD(value, AC97_CONTROL_STATUS_CONTROL, ENABLED);
	SmWrite32(AC97_CONTROL_STATUS, value);

	/*
	 * We're failing here but one should be able to use the codec
	 * without firmware according to the manual
	 */
	ret = sm501_load_firmware(audio_dev, FIRMWARE);
	if (ret < 0) {
		dev_err(dev, "Error while loading firmware (%d)\n", ret);
		if(retry++ < 5) /* If load error, retry it */
			goto restart;
		goto err;
	}

	audio_dev->firmware_loaded = 1;
	//monk add below,y not init the lock here
	spin_lock_init(&audio_dev->lock);

	dev_info(dev, "SM501 AC97 Found\n");
	return 0;

err_alloc:
	kfree(audio_dev);
err:
	return ret;
}

static void snd_sm501_ac97_remove(struct platform_device *pdev)
{
	unsigned int value;

	kfree(audio_dev);

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
	sm501_set_gate(value);
}

static int snd_sm501_ac97_suspend (struct platform_device *pdev) {
    
    printk("SM502 AC97 Suspend....\n");

    unsigned int value;
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
    sm501_set_gate(value);
    audio_dev->firmware_loaded = 0;
}


static int snd_sm501_ac97_resume (struct platform_device *pdev) {
    struct device   *dev = &pdev->dev;
    struct resource *res;
    int ret = 0;
    int res_len;
    unsigned long value;

    printk("SM502 AC97 Resume\n");

    //Reset AC97 codec

    /* Put 8051 in reset state. */
    value = SmRead32(U8051_RESET);
    value = SET_FIELD(value, U8051_RESET_CONTROL, RESET);
    SmWrite32(U8051_RESET, value);

    /* Reset GPIO lines. */
    value = SmRead32(GPIO_CONTROL_LOW);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_28, GPIO);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_27, GPIO);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_26, GPIO);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_25, GPIO);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_24, GPIO);
    SmWrite32(GPIO_CONTROL_LOW, value);

    /*Force GPIO28/SDIN GPIO27/SDOUT GPIO25/SYNC to low*/
    value = SmRead32(GPIO_DATA_DIRECTION_LOW);
    value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_28, OUTPUT);
    value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_27, OUTPUT);
    value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_25, OUTPUT);
    SmWrite32(GPIO_DATA_DIRECTION_LOW, value);


    value = SmRead32(GPIO_DATA_LOW);
    value = SET_FIELD(value, GPIO_DATA_LOW_28, LOW);
    value = SET_FIELD(value, GPIO_DATA_LOW_27, LOW);
    value = SET_FIELD(value, GPIO_DATA_LOW_25, LOW);
    SmWrite32(GPIO_DATA_LOW, value);


    /*Force GPIO24/AC97 Reset to low*/
    value = SmRead32(GPIO_DATA_DIRECTION_LOW);
    value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_24, OUTPUT);
    SmWrite32(GPIO_DATA_DIRECTION_LOW, value);


    value = SmRead32(GPIO_DATA_LOW);
    value = SET_FIELD(value, GPIO_DATA_LOW_24, LOW);
    SmWrite32(GPIO_DATA_LOW, value);
    msleep(10);

    /*Force GPIO24/AC97 Reset to high*/
    value = SmRead32(GPIO_DATA_DIRECTION_LOW);
    value = SET_FIELD(value, GPIO_DATA_DIRECTION_LOW_24, OUTPUT);
    SmWrite32(GPIO_DATA_DIRECTION_LOW, value);


    value = SmRead32(GPIO_DATA_LOW);
    value = SET_FIELD(value, GPIO_DATA_LOW_24, HIGH);
    SmWrite32(GPIO_DATA_LOW, value);


    ////////////////////
    value = SmRead32(GPIO_CONTROL_LOW);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_28, AC97_RX_I2S);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_27, AC97_TX_I2S);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_26, AC97_CLOCK_I2S);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_25, AC97_SYNC_I2S);
    //	value = SET_FIELD(value, GPIO_CONTROL_LOW_24, SM501_AC97_RESET);
    value = SET_FIELD(value, GPIO_CONTROL_LOW_24, AC97_RESET);
    SmWrite32(GPIO_CONTROL_LOW, value);

    /* Set arbitartion control */
    value = SmRead32(ARBITRATION_CONTROL);
    value = SET_FIELD(value, ARBITRATION_CONTROL_DMA, PRIORITY_1);
    SmWrite32(ARBITRATION_CONTROL, value);

    /* Enable interrupts */
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

    value = SmRead32(AC97_CONTROL_STATUS);
    value = SET_FIELD(value, AC97_CONTROL_STATUS_CONTROL, ENABLED);
    SmWrite32(AC97_CONTROL_STATUS, value);

    /*
     * We're failing here but one should be able to use the codec
     * without firmware according to the manual
     */
    ret = sm501_load_firmware(audio_dev, FIRMWARE);
    if (ret < 0) {
        dev_err(dev, "Error while loading firmware (%d)\n", ret);
    }

    audio_dev->firmware_loaded = 1;

    return 0;
}



struct snd_soc_dai sm501_ac97_dai[] = {{
	.name = DRV_NAME,
	.id = 0,
	.type = SND_SOC_DAI_AC97,
	.probe = snd_sm501_ac97_probe,
	.remove = snd_sm501_ac97_remove,
	.suspend = snd_sm501_ac97_suspend,
        .resume = snd_sm501_ac97_resume,
	.playback = {
		.stream_name = "AC97 Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_48000, /* SNDRV_PCM_RATE_8000_48000,*/
		/* change to be with be systems */
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.capture = {
		.stream_name = "AC97 Capture",
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_48000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = {
		.hw_params = snd_sm501_ac97_hw_params,
		.prepare   = snd_sm501_ac97_prepare,
	},
},
#if 0
#warning Put mic here
{}
#endif
};
EXPORT_SYMBOL_GPL(sm501_ac97_dai);

MODULE_AUTHOR("Arnaud Patard <apatard@mandriva.com>");
MODULE_DESCRIPTION("ASoC AC97 driver for SM501 chip");
MODULE_LICENSE("GPL");
