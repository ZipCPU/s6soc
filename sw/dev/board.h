////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	board.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To define the interfaces to the peripherals on the board, as
//		given by the ZipCPU's view of the board.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015-2016, Gisselquist Technology, LLC
//
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of  the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory, run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
//
#ifndef	BOARD_H
#define	BOARD_H

// GPIO PINS
//   first the outputs ...
#define	GPO_SDA		0x000001
#define	GPO_SCL		0x000002
#define	GPO_MOSI	0x000004
#define	GPO_SCK		0x000008
#define	GPO_SS		0x000010
//   then the inputs.
#define	GPI_SDA		0x010000
#define	GPI_SCL		0x020000
#define	GPI_MISO	0x040000

#define	GPOSETV(PINS)	((PINS)|((PINS)<<16))
#define	GPOCLRV(PINS)	((PINS)<<16)

// Interrupts
#define	INT_ENABLE	0x80000000
#define	INT_BUTTON	0x001
#define	INT_BUSERR	0x002 // Kind of useless, a buserr will kill us anyway
#define	INT_SCOPE	0x004
#define	INT_TIMER	0x010
//#define INT_WATCHDOG	0x020	// Catching a watchdog/reset interrupt makes no sense
#define	INT_UARTRX	0x040
#define	INT_UARTTX	0x080
#define	INT_KEYPAD	0x100
#define	INT_AUDIO	0x200
#define	INT_GPIO	0x400
// #define	INT_FLASH	0x800	// Not available due to lack of space
#define	INT_ENABLEV(IN)		(INT_ENABLE|((IN)<<16))
#define	INT_DISABLEV(IN)	((IN)<<16)
#define	INT_CLEAR(IN)		(IN)
#define	INT_CLEARPIC	0x7fff7fff
#define	INT_DALLPIC	0x7fff0000

// Clocks per second, for use with the timer
#define	TM_ONE_SECOND	80000000
#define	TM_REPEAT	0x80000000

typedef	struct	{
	int		io_pic;
	unsigned	*io_buserr;
	int		io_timer, io_watchdog;
	unsigned	io_pwm_audio;
	unsigned	io_spio; // aka keypad, buttons, and keyboard
	unsigned	io_gpio;
	unsigned	io_uart;
	unsigned	io_version;
} IOSPACE;


#define	WBSCOPE_NO_RESET	0x80000000
#define	WBSCOPE_MANUAL	WBSCOPE_TRIGGER
//
#define	WBSCOPE_STOPPED		0x40000000
#define	WBSCOPE_TRIGGERED	0x20000000
#define	WBSCOPE_PRIMED		0x10000000
#define	WBSCOPE_TRIGGER	       (0x08000000|WBSCOPE_NO_RESET)
#define	WBSCOPE_DISABLED	0x04000000
#define	WBSCOPE_DISABLE		0x04000000	// Disable the scope trigger
#define	WBSCOPE_RZERO		0x02000000	// Unused,true if ptd at begning
#define	WBSCOPE_LGLEN(A)	((A>>20)&0x01f)
#define	WBSCOPE_LENGTH(A)	(1<<(WBSCOPE_LGLEN(A)))

typedef	struct WBSCOPE_S {
	unsigned	s_ctrl, s_data;
} WBSCOPE;

#define	IOADDR		0x000400
#define	SCOPEADDR	0x000800
// #define FCTLADDR	0x000c00 // Flash control, depends upon write capability
#define	BKRAM		(void *)0x004000
#define	FLASH		(void *)0x1000000
#define	SDRAM		(void *)0
#define	RAMSZ		(RAMADDR)
#define	FLASHSZ		(FLASHADDR)
#define	MEMLEN		0x04000
#define	FLASHLEN	0x1000000
#define	RESET_ADDR	0x1200000

#define	CLOCKFREQHZ	80000000
#define	CLOCKFREQ_HZ	CLOCKFREQHZ

static	volatile IOSPACE *const _sys   = (IOSPACE *)IOADDR;
#define	_ZIP_HAS_WBUARTRX
#define	_uartrx		_sys->io_uart
#define	_ZIP_HAS_LONELY_UART
#define	LONELY_UART
#define	_uart		_sys->io_uart

static	volatile WBSCOPE *const _scope = (WBSCOPE *)SCOPEADDR;

#define	SYSTIMER	_sys->io_timer
#define	SYSPIC		_sys->io_pic

#define	valid_ram_region(PTR,LN) (((int)(PTR)>=RAMADDR)&&((int)(PTR+LN)<RAMADDR+RAMSZ))
#define	valid_flash_region(PTR,LN) (((int)(PTR)>=FLASHADDR)&&((int)(PTR+LN)<FLASHADDR+FLASHSZ))
#define	valid_mem_region(PTR,LN)	((valid_ram_region(PTR,LN))||(valid_flash_region(PTR,LN)))

#endif
