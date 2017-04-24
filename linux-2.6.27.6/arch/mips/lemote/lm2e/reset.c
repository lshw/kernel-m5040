/*
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Copyright (C) 2007 Lemote, Inc. & Institute of Computing Technology
 * Author: Fuxin Zhang, zhangfx@lemote.com
 */
#include <linux/pm.h>

#include <asm/reboot.h>

static void loongson2e_restart(char *command)
{
#ifdef CONFIG_32BIT
	*(unsigned long *)0xbfe00104 &= ~(1 << 2);
	*(unsigned long *)0xbfe00104 |= (1 << 2);
#else
	*(unsigned long *)0xffffffffbfe00104 &= ~(1 << 2);
	*(unsigned long *)0xffffffffbfe00104 |= (1 << 2);
#endif
	__asm__ __volatile__("jr\t%0"::"r"(0xbfc00000));
}


static void delay(void)
{
	volatile int i;
	for (i=0; i<0x10000; i++);
}
static void loongson2e_halt(void)
{
#ifdef CONFIG_32BIT
	*(unsigned short*)0xbfd0b000 = 0x8100;
	delay();
	*(unsigned short *)0xbfd0b004 = 0x2800;
	delay();
	*(unsigned int *)0xbfe00148 = 0x120002;
#else
	*(unsigned short *)0xffffffffbfd0b000 = 0x8100;
	delay();
	*(unsigned short *)0xffffffffbfd0b004 = 0x2800;
	delay();
	*(unsigned int *)0xffffffffbfe00148 = 0x120002;
#endif
}

static void loongson2e_power_off(void)
{
	loongson2e_halt();
}

void mips_reboot_setup(void)
{
	_machine_restart = loongson2e_restart;
	_machine_halt = loongson2e_halt;
	pm_power_off = loongson2e_power_off;
}
