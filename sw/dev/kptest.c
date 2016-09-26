////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	kptest.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To test and demonstrate that the keypad works.
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
#include "asmstartup.h"
#include "board.h"
#include "keypad.h"

void    txchr(char v) {
        volatile IOSPACE        *sys = (IOSPACE *)IOADDR;
        if (v < 10)
                return;
        v &= 0x07f;
        sys->io_pic = INT_UARTTX;
        while((sys->io_pic&INT_UARTTX)==0)
                ;
        sys->io_uart = v;
}

void    txstr(const char *str) {
        const char *ptr = str;
        while(*ptr) {
                txchr(*ptr++);
        }
}

void entry(void) {
	register IOSPACE	*sys = (IOSPACE *)0x0100;

	sys->io_pic = 0x07fffffff; // Acknowledge and turn off all interrupts
	sys->io_spio = 0x0f0;
	sys->io_tima = 100000 | TM_REPEAT;

	txstr("Press any keypad button for test.\r\n");

	while(1) {
		int	ch;
		while(0 == (sys->io_pic & INT_KEYPAD))
			;
		sys->io_pic = INT_KEYPAD | INT_TIMA;
		// Wait 5 ms
		for(int i=0; i<5; i++) {
			while(0 == (sys->io_pic & INT_TIMA))
				;
		}
		sys->io_spio = 0x011;
		ch = keypadread();
		if (ch < 0)
			; // txstr("Unknown key pressed or error\n");
		else if (ch < 10)
			txchr(ch+'0');
		else if (ch == 15)
			txstr("F\r\n");
		else if (ch < 15)
			txchr(ch+'A'-10);
		else txstr("Unknown key pressed\n");
		keypad_wait_for_release();
		sys->io_spio = 0x010;
	}
}
