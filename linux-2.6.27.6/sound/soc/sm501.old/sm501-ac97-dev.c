/*
 * sm501-ac97-dev.c  --  SoC audio for gdium
 *
 * Based on smdk2443_wm9710.c
 *
 * Arnaud Patard <apatard@mandriva.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <asm/gpio.h>

#include "../codecs/ac97.h"
#include "sm501-pcm.h"
#include "sm501-ac97.h"

#define SPEAKER_PWM		2
#define SPEAKER_PWM_PERIOD	(1000000000/20000) /* 20 kHz */

#define MUTE_GPIO		192+0 /* pin 32 is mute */
#define AMP_MUTE		1
#define AMP_UNMUTE		0

static struct snd_soc_dai_link gdium_dai[] = {
{
	.name = "AC97",
	.stream_name = "AC97 HiFi",
	.cpu_dai = &sm501_ac97_dai[0],
	.codec_dai = &ac97_dai,
},
};

static struct snd_soc_machine gdium_machine = {
	.name = "lyn",
	.dai_link = gdium_dai,
	.num_links = ARRAY_SIZE(gdium_dai),
};

static struct snd_soc_device gdium_snd_devdata = {
	.machine = &gdium_machine,
	.platform = &sm501_soc_platform,
	.codec_dev = &soc_codec_dev_ac97,
};

static struct platform_device *gdium_snd_ac97_device;




static int sm501_snd_probe(struct platform_device *pdev)
{

	int ret;

	gdium_snd_ac97_device = platform_device_alloc("soc-audio", 1);
	if (!gdium_snd_ac97_device)
		return -ENOMEM;

	gdium_snd_ac97_device->num_resources = pdev->num_resources;
	gdium_snd_ac97_device->resource = pdev->resource;
    gdium_snd_ac97_device->dev.parent = &pdev->dev;

	platform_set_drvdata(gdium_snd_ac97_device,
				&gdium_snd_devdata);
	gdium_snd_devdata.dev = &gdium_snd_ac97_device->dev;
	ret = platform_device_add(gdium_snd_ac97_device);

	if (ret)
		platform_device_put(gdium_snd_ac97_device);

	return ret;
}

static void sm501_snd_remove(struct platform_device * pdev)
{
	platform_device_unregister(gdium_snd_ac97_device);
}
static struct platform_driver sm501_ac97_driver = {
			.remove		= sm501_snd_remove,
			.probe		= sm501_snd_probe,
			.driver		= {
			.name	= "sm501-audio",
				.owner	= THIS_MODULE,
			},
};

static int __devinit sm501_snd_init(void)
{
    return platform_driver_register(&sm501_ac97_driver);
}

static void __exit sm501_snd_cleanup(void)
{
	platform_driver_unregister(&sm501_ac97_driver);
}


module_init(sm501_snd_init);
module_exit(sm501_snd_cleanup);



/* Module information */
MODULE_AUTHOR("Arnaud Patard <apatard@mandriva.com>");
MODULE_DESCRIPTION("ALSA SoC Gdium");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:gdium-audio");
