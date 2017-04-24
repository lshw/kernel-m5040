/*
 * Copyright (C) 2007 Lemote Inc. & Insititute of Computing Technology
 * Author: Fuxin Zhang, zhangfx@lemote.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/irq_cpu.h>
#include <asm/i8259.h>
#include <asm/mipsregs.h>
#include <asm/delay.h>
#include <asm/mips-boards/bonito64.h>

#define	BONITO_INT_BIT_GPIO0		(1 << 0)
#define	BONITO_INT_BIT_GPIO1		(1 << 1)
#define	BONITO_INT_BIT_GPIO2		(1 << 2)
#define	BONITO_INT_BIT_GPIO3		(1 << 3)
#define	BONITO_INT_BIT_PCI_INTA		(1 << 4)
#define	BONITO_INT_BIT_PCI_INTB		(1 << 5)
#define	BONITO_INT_BIT_PCI_INTC		(1 << 6)
#define	BONITO_INT_BIT_PCI_INTD		(1 << 7)
#define	BONITO_INT_BIT_PCI_PERR		(1 << 8)
#define	BONITO_INT_BIT_PCI_SERR		(1 << 9)
#define	BONITO_INT_BIT_DDR		(1 << 10)
#define	BONITO_INT_BIT_INT0		(1 << 11)
#define	BONITO_INT_BIT_INT1		(1 << 12)
#define	BONITO_INT_BIT_INT2		(1 << 13)
#define	BONITO_INT_BIT_INT3		(1 << 14)

#define	BONITO_INT_TIMER_OFF		7
#define	BONITO_INT_BONITO_OFF		6
#define	BONITO_INT_UART_OFF			3
#define	BONITO_INT_I8259_OFF		2

/****************************************************************/

static void godson2f_timer_dispatch(void)
{
	/* place the godson2f timer interrupt on 23 */
	do_IRQ(MIPS_CPU_IRQ_BASE + BONITO_INT_TIMER_OFF);
	return;
}

static void godson2f_bonito_dispatch(void)
{
	int int_status;
	int i = 0;

	/* place the other interrupt on bit6 for bonito, inclding PCI and so on */
	int_status = BONITO_INTISR & BONITO_INTEN;
	
	for(i = 0; (i < 10) && int_status; i++){
		if(int_status & (1 << i)){
			if(i == 10)
				printk("ddr int.\n");
#if 0
			if(int_status & 0x00000004)
				printk("cpu gpio int %08x\n",int_status);
			if(int_status & 0x000000ff)
#endif
				do_IRQ(BONITO_IRQ_BASE + i);
			int_status &= ~(1 << i);
		}
	}
	
	return;
}

static void godson2f_int3_dispatch(void)
{
	do_IRQ(21);
	
	return;
}

static void godson2f_int2_dispatch(void)
{
	do_IRQ(20);
	
	return;
}

static void godson2f_int0_dispatch(void)
{
		do_IRQ(18);
}
static void godson2f_int1_dispatch(void)
{
	/* place the godson2f uart interrupt on int1 */
	//do_IRQ(MIPS_CPU_IRQ_BASE + BONITO_INT_UART_OFF);
//	int_status = BONITO_INTISR & BONITO_INTEN;
//	if(int_status & BONITO_INT_BIT_INT1)
	do_IRQ(19);
}

static void i8259_irqdispatch(void)
{
	int irq, isr;
	extern unsigned int cached_irq_mask;

	if((BONITO_INTISR & BONITO_INTEN) & BONITO_INT_BIT_INT0) {

		isr = inb(0x20) | (inb(0xa0) << 8);
		isr &= ~0x4; // irq2 for cascade
		isr &= ~cached_irq_mask;
		irq = ffs(isr) - 1;

		if(irq >= 0) {
			do_IRQ(irq);
		} else {
			spurious_interrupt();
		}
	}
}

static void godson2f_steer1_dispatch(void)
{
	printk(KERN_INFO "godson2f steer1 is not used yet.\n");
	return;
}

static void godson2f_steer0_dispatch(void)
{
	printk(KERN_INFO "godson2f steer0 is not used yet.\n");
	return;
}


asmlinkage void plat_irq_dispatch(void)
{
	unsigned int pending = read_c0_cause() & read_c0_status() & ST0_IM;
		//printk(">>>>check your mother board.%08x,%08x\n",read_c0_cause(),read_c0_status());

	if(pending & CAUSEF_IP7){
		godson2f_timer_dispatch();
	}else if(pending & CAUSEF_IP6){ /*nb*/
#ifdef CONFIG_PROFILING
		do_IRQ(MIPS_CPU_IRQ_BASE + 6);
#endif
		godson2f_bonito_dispatch();
	}else if(pending & CAUSEF_IP5){
		godson2f_int3_dispatch();
	}else if(pending & CAUSEF_IP4){
		godson2f_int2_dispatch();
	}else if(pending & CAUSEF_IP3){ /*cpu uart*/
		godson2f_int1_dispatch();
	}else if(pending & CAUSEF_IP2){ /*sb*/
		godson2f_int0_dispatch();
		//i8259_irqdispatch();
	}else if(pending & CAUSEF_IP1){
		godson2f_steer1_dispatch();
	}else if(pending & CAUSEF_IP0){
		godson2f_steer0_dispatch();
	}else{
		spurious_interrupt();
	}
	return;
}

static struct irqaction cascade_irqaction = {
	.handler = no_action,
	.mask = CPU_MASK_NONE,
	.name = "cascade",
};

irqreturn_t ip6_action(int cpl, void *dev_id, struct pt_regs *regs)
{
	return IRQ_HANDLED;
}

static struct irqaction ip6_irqaction = {
	.handler = ip6_action,
	.mask = CPU_MASK_NONE,
	.name = "cascade",
	.flags = IRQF_SHARED,
};


void __init arch_init_irq(void)
{
	extern void bonito_irq_init(void);

	/*
	 * Clear all of the interrupts while we change the able around a bit.
	 * int-handler is not on bootstrap
	 */
	clear_c0_status(ST0_IM | ST0_BEV);
	local_irq_disable();
	
#if	0
	/* most bonito irq should be level triggered */
	BONITO_INTEDGE = BONITO_ICU_SYSTEMERR | BONITO_ICU_MASTERERR
	    | BONITO_ICU_RETRYERR | BONITO_ICU_MBOXES;
#endif

	/* setup cs5536 as high level */
//	BONITO_INTEDGE |= (1 << 11 | 1 << 12);
	BONITO_INTPOL  = (1 << 11 | 1 << 12 | 1 << 13 | 1 << 14 | 1 << 0| 1 << 1| 1 << 2);
//	BONITO_INTEDGE &= ~(1 << 11 | 1 << 12);
//	BONITO_INTEDGE |= (1 << 11 | 1 << 12);

	/* no steer */
	BONITO_INTSTEER = 0;
	/*
	 * Mask out all interrupt by writing "1" to all bit position in
	 * the interrupt reset reg.
	 */
	BONITO_INTENCLR = ~0;

	/* init all controller
	 *   0-15         ------> i8259 interrupt
	 *   16-23        ------> mips cpu interrupt
	 *   32-63        ------> bonito irq
	 */
	
	/* Sets the first-level interrupt dispatcher. */
	mips_cpu_irq_init();

	init_i8259_irqs();
	bonito_irq_init();
	
	/* setup bonito interrupt */
	//setup_irq(MIPS_CPU_IRQ_BASE + BONITO_INT_BONITO_OFF, &cascade_irqaction);
#ifdef CONFIG_PROFILING
	setup_irq(MIPS_CPU_IRQ_BASE + BONITO_INT_BONITO_OFF, &ip6_irqaction);
	setup_irq(MIPS_CPU_IRQ_BASE + 2, &ip6_irqaction);
#else
	setup_irq(MIPS_CPU_IRQ_BASE + BONITO_INT_BONITO_OFF, &cascade_irqaction);
#endif
//printk("MIPS_CPU_IRQ_BASE:%d\n",MIPS_CPU_IRQ_BASE);
//printk("cpu gpio:%08x,%08x\n",*(unsigned int *)0xffffffffbfe00120,*(unsigned int *)0xffffffffbfe0011c);
	/* 8259 irq at IP2 */
//	setup_irq(MIPS_CPU_IRQ_BASE + BONITO_INT_I8259_OFF, &cascade_irqaction);

}
