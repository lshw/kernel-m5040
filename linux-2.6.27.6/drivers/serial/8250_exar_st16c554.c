/*
 *  linux/drivers/serial/8250_exar.c
 *
 *  Written by Paul B Schroeder < pschroeder "at" uplogix "dot" com >
 *  Based on 8250_boca.
 *
 *  Copyright (C) 2005 Russell King.
 *  Data taken from include/asm-i386/serial.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/serial_8250.h>
/*
#define PORT(_base,_irq)				\
	{						\
		.iobase		= _base,		\
		.irq		= _irq,			\
		.uartclk	= 1843200,		\
		.iotype		= UPIO_PORT,		\
		.flags		= UPF_BOOT_AUTOCONF,	\
	}
*/
#define PORT(_base,_irq)				\
	{						\
		.mapbase  = (resource_size_t)_base,		\
		.irq		= _irq,			\
		.uartclk	= 14745600,		\
		.iotype		= UPIO_MEM,		\
		.flags		= (UPF_BOOT_AUTOCONF|UPF_IOREMAP),	\
		.regshift = 0, \
	}
static struct plat_serial8250_port exar_data[] = {
	PORT(0x1e000000, 18),
	PORT(0x1e000020, 19),
	PORT(0x1e000040, 20),
	PORT(0x1e000060, 21),
	PORT(0x1e0000a0, 32),
	PORT(0x1e0000c0, 33),
	{ },
};

static struct platform_device exar_device = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_EXAR_ST16C554,
	.dev			= {
		.platform_data	= exar_data,
	},
};

static int __init exar_init(void)
{
	return platform_device_register(&exar_device);
}

module_init(exar_init);

MODULE_AUTHOR("Paul B Schroeder");
MODULE_DESCRIPTION("8250 serial probe module for Exar cards");
MODULE_LICENSE("GPL");
