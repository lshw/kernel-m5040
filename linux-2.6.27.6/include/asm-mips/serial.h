/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999 by Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#ifndef _ASM_SERIAL_H
#define _ASM_SERIAL_H


/*
 * This assumes you have a 1.8432 MHz clock for your UART.
 *
 * It'd be nice if someone built a serial card with a 24.576 MHz
 * clock, since the 16550A is capable of handling a top speed of 1.5
 * megabits/second; but this requires the faster clock.
 */
#define BASE_BAUD (1843200 / 16)

/* Standard COM flags (except for COM4, because of the 8514 problem) */
#ifdef CONFIG_SERIAL_DETECT_IRQ
#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST | ASYNC_AUTO_IRQ)
#define STD_COM4_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_AUTO_IRQ)
#else
#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)
#define STD_COM4_FLAGS ASYNC_BOOT_AUTOCONF
#endif

#ifdef CONFIG_HAVE_STD_PC_SERIAL_PORT
#define STD_SERIAL_PORT_DEFNS                   \
        /* UART CLK   PORT IRQ     FLAGS        */                      \
        { 0, BASE_BAUD, 0x3F8, 4, STD_COM_FLAGS },      /* ttyS0 */     \
        { 0, BASE_BAUD, 0x2F8, 3, STD_COM_FLAGS },      /* ttyS1 */     \
        { 0, BASE_BAUD, 0x3E8, 4, STD_COM_FLAGS },      /* ttyS2 */     \
        { 0, BASE_BAUD, 0x2E8, 3, STD_COM4_FLAGS },     /* ttyS3 */

#else /* CONFIG_HAVE_STD_PC_SERIAL_PORTS */
#define STD_SERIAL_PORT_DEFNS
#endif /* CONFIG_HAVE_STD_PC_SERIAL_PORTS */

#define SERIAL_PORT_DFNS                                \
        STD_SERIAL_PORT_DEFNS  

#endif /* _ASM_SERIAL_H */
