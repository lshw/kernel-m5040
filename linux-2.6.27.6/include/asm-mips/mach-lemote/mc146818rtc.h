/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1998, 2001, 03, 07 by Ralf Baechle (ralf@linux-mips.org)
 *
 * RTC routines for PC style attached Dallas chip.
 */
#ifndef __ASM_MACH_LEMOTE_MC146818RTC_H
#define __ASM_MACH_LEMOTE_MC146818RTC_H

#include <linux/io.h>

#define RTC_PORT(x)	(0x70 + (x))
#define RTC_IRQ		8
#if 1
#define I2C_SINGLE 0
int tgt_i2cread(int type,unsigned char *addr,int addrlen,unsigned char reg,unsigned char *buf,int count);
int tgt_i2cwrite(int type,unsigned char *addr,int addrlen,unsigned char reg,unsigned char *buf,int count);
static inline unsigned char CMOS_READ(unsigned char addr)
{
    unsigned char val;
    unsigned char tmp1,tmp2;
	volatile int tmp;
	unsigned char value;
	char i2caddr[] = {(unsigned char)0xd0};
	if(addr >= 0x0a)
		return 0;
	value = value | 0x20;
	tgt_i2cread(I2C_SINGLE,i2caddr,1,addr,&val,1);
	tmp1 = ((val >> 4) & 0x0f)*10;
	tmp2  = val & 0x0f;
	val = tmp1 + tmp2;
    return val;
}

static inline void CMOS_WRITE(unsigned char val, unsigned char addr)
{
	char a;
  	unsigned char tmp1,tmp2;
	volatile int tmp;
	char i2caddr[] = {(unsigned char)0xd0};
	tmp1 = (val / 10) << 4;
	tmp2  = (val % 10);
	val = tmp1 | tmp2;
	if(addr >= 0x0a)
		return 0;
	{
		unsigned char value;
		value = value | 0x20;
		tgt_i2cwrite(I2C_SINGLE,i2caddr,1,addr,&val,1);
	}
}

#else
static inline unsigned char CMOS_READ(unsigned long addr)
{
	outb_p(addr, RTC_PORT(0));
	return inb_p(RTC_PORT(1));
}

static inline void CMOS_WRITE(unsigned char data, unsigned long addr)
{
	outb_p(addr, RTC_PORT(0));
	outb_p(data, RTC_PORT(1));
}
#endif
#define RTC_ALWAYS_BCD	0

#ifndef mc146818_decode_year
#define mc146818_decode_year(year) ((year) < 70 ? (year) + 2000 : (year) + 1970)
#endif

#endif /* __ASM_MACH_LEMOTE_MC146818RTC_H */
