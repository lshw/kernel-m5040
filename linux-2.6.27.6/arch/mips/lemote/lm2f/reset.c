/*
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Copyright (C) 2007 Lemote, Inc. & Institute of Computing Technology
 * Author: Fuxin Zhang, zhangfx@lemote.com
 */

#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/reboot.h>
#include <asm/system.h>

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pm.h>
#include <linux/delay.h>
extern void _wrmsr(u32 reg, u32 hi, u32 lo);
extern void _rdmsr(u32 reg, u32 *hi, u32 *lo);
void prom_printf(char *fmt, ...);
static void loongson2e_restart(char *command)
{
#ifdef CONFIG_LEMOTE_FULONG2F
	*(volatile unsigned long *)0xffffffffbfe00120 &= ~(1 << 3);
	*(volatile unsigned long *)0xffffffffbfe0011c &= ~(1 << 3);
/*	u32 hi, lo;
	_rdmsr(0xe0000014, &hi, &lo);
	lo |= 0x00000001;
	_wrmsr(0xe0000014, hi, lo);*/
#else

#ifdef CONFIG_32BIT
	*(unsigned long *)0xbfe00120 &= ~(1 << 3);
	*(unsigned long *)0xbfe0011c &= ~(1 << 3);
#else
	*(volatile unsigned long *)0xffffffffbfe00120 &= ~(1 << 3);
	*(volatile unsigned long *)0xffffffffbfe0011c &= ~(1 << 3);
#endif
#endif
	printk("Hard reset not take effect!!\n");
	__asm__ __volatile__ (
					".long 0x3c02bfc0\n"
					".long 0x00400008\n"
					:::"v0"
					);
}

void sm501_date_gpio(unsigned int gpio, unsigned char mode); 
void sm501_dir_gpio(unsigned int gpio, unsigned char mode); 

static void delay(void)
{
	volatile int i;
	for (i=0; i<0x10000; i++);
}
static void loongson2e_halt(void)
{
	int i,j; //add by zhao

#if 0
	volatile int *gpio_date = 0xffffffffb6010004;
	volatile int *gpio_dir = 0xffffffffb601000c;
	int tmp;
	prom_printf("-----test\n");
	tmp = *gpio_dir;
	prom_printf("--1---tmpdir:%08x\n",tmp);
	*gpio_dir = tmp|1<<22;
	prom_printf("--2---tmpdir:%08x\n",*gpio_dir);
	tmp = *gpio_date;
	prom_printf("--3---tmpdir:%08x\n",tmp);
	*gpio_date = tmp& ~(1<<22);
	prom_printf("--4---tmpdir:%08x\n",*gpio_date);
#endif

   sm501_dir_gpio(1,1);
   sm501_date_gpio(1,0);

/*****************add by zhao*************************/
	for(i = 0; i < 1000; i++)
	{
	for(j = 0; j < 1000; j++)
			;
	}
	delay();
  sm501_date_gpio(1,1);
/*****************************************************/
  //sm501_dir_gpio(54,1);
//sm501_date_gpio(54,0);
#if 0
#ifdef CONFIG_LEMOTE_FULONG2F
#ifdef CONFIG_32BIT
	u32 base;
#else
	u64 base;
#endif
	u32 hi, lo, val;
	
	_rdmsr(0x8000000c, &hi, &lo);
#ifdef CONFIG_32BIT
	base = (lo & 0xff00) | 0xbfd00000;
#else
	base = (lo & 0xff00) | 0xffffffffbfd00000ULL;
#endif
	val = *(volatile unsigned int *)(base + 0x04);
	val = (val & ~(1 << (16 + 13))) | (1 << 13);
	delay();
	*(__volatile__ u32 *)(base + 0x04) = val;
	delay();
	val = (val & ~(1 << (13))) | (1 << (16 + 13));
	delay();
	*(__volatile__ u32 *)(base + 0x00) = val;
	delay();
#else
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
#endif
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
