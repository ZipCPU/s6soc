////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	helloworld.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	A very simple program.  This program tests the LEDs, buttons,
//		and UART.  Specifically, if run, this program will cycle through
//	the LEDs at one change per second.  If one button is pressed, it will
//	cycle through turning one LED off once per second while turning the 
//	rest on.  If the other button is pressed, two LED's will never turn on.
//
//	To test the UART, the message "Hello, world!" will be sent to the UART
//	at the top of each second.
//
//	This program is simple.  Although it uses the interrupt controller,
//	interrupts are disabled throughout the program.  The interrupt
//	controller is only used to determine when events have taken place.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015-2017, Gisselquist Technology, LLC
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
#include "asmstartup.h"
#include "board.h"

const char msg[] = "Hello, world!\r\n";

void entry(void) {
	register IOSPACE	*sys = (IOSPACE *)IOADDR;
	int	ledset = 0;

	sys->io_spio = 0x0f0;

	/// Turn off timer B
	sys->io_timb = 0;

	while(1) {
		const char	*ptr;
		sys->io_tima = TM_ONE_SECOND; // Ticks per second, 80M
		sys->io_pic  = 0x07fffffff; // Acknowledge and turn off all ints

		ptr = msg;
		while(*ptr) {
			unsigned iv = *(unsigned char *)ptr++;

			// Wait while our transmitter is busy
			while((sys->io_pic & INT_UARTTX)==0)
				;
			sys->io_uart = iv; // Transmit our character
			sys->io_pic  = INT_UARTTX; // Clear the int flag
		}

		// Now, wait for the top of the second
		while((sys->io_pic & INT_TIMA)==0)
			;

		ledset <<= 1;
		ledset &= 15;
		if (ledset == 0)
			ledset = 1;
		ledset |= 0xf0;

		int	btn = sys->io_spio;
		if (btn&0x10)
			sys->io_spio = ledset^0x0f;
		else if (btn&0x20)
			sys->io_spio = ledset&0x0f5;
		else
			sys->io_spio = ledset;
	}
}

