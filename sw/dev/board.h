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
#define	INT_RTC		0x008 // May not be available, due to lack of space
#define	INT_TIMA	0x010
#define	INT_TIMB	0x020
#define	INT_UARTRX	0x040
#define	INT_UARTTX	0x080
#define	INT_KEYPAD	0x100
#define	INT_AUDIO	0x200
#define	INT_GPIO	0x400
// #define	INT_FLASH	0x800	// Not available due to lack of space
#define	INT_ENABLEV(IN)		(INT_ENABLE|((IN)<<16))
#define	INT_DISABLEV(IN)	(INT_ENABLE|((IN)<<16))
#define	INT_CLEAR(IN)		(IN)

// Clocks per second, for use with the timer
#define	TM_ONE_SECOND	80000000
#define	TM_REPEAT	0x80000000

typedef	struct	{
	volatile int		io_pic;
	volatile unsigned	*io_buserr;
	volatile int		io_tima, io_timb;
	volatile unsigned	io_pwm_audio;
	volatile unsigned	io_spio; // aka keypad, buttons, and keyboard
	volatile unsigned	io_gpio;
	volatile unsigned	io_uart;
	volatile unsigned	io_version;
} IOSPACE;

typedef	struct {
	volatile unsigned	s_control, s_data;
} SCOPE;

typedef	struct	{
	volatile unsigned	f_crc, f_far_maj, f_far_min, f_fdri,
			f_fdro, f_cmd, f_ctl, f_mask,
			f_stat, f_lout, f_cor1, f_cor2,
			f_pwrdn, f_flr, f_idcode, f_cwdt,
			f_hcopt, f_csbo, f_gen1, f_gen2,
			f_gen3, f_gen4, f_gen5, f_mode,
			f_gwe, f_mfwr, f_cclk, f_seu, f_exp, f_rdbk,
			f_bootsts, f_eye, f_cbc;
} FPGACONFIG;

typedef	struct	{
	volatile unsigned	c_clock, c_timer, c_stopwatch, c_alarm;
} RTCCLOCK;

#define	IOADDR		0x000100
#define	SCOPEADDR	0x000200
// #define FCTLADDR	0x000300 // Flash control, depends upon write capability
#define	CONFIGADDR	0x000400
// #define RTCADDR	0x000800 // Disabled for lack of space on device
#define	RAMADDR		0x002000
#define	FLASHADDR	0x400000
#define	RESET_ADDR	0x480000

#endif
