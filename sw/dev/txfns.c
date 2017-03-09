////////////////////////////////////////////////////////////////////////////////
//
// Filename:	txfns.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:
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
#include "board.h"
#include "txfns.h"

void	txchr(char ch);
void	txval(int val);
void	txstr(const char *str);

void	txchr(char val) {
	volatile IOSPACE *const sys = _sys;
	unsigned v = (unsigned char)val;

	// To read whether or not the transmitter is ready, you must first
	// clear the interrupt bit.
	sys->io_pic = INT_UARTTX;
	// If the interrupt bit sets itself again immediately, the transmitter
	// is ready.  Otherwise, wait until the transmitter becomes ready.
	while((sys->io_pic&INT_UARTTX)==0)
		;
	sys->io_uart = (v&0x0ff);
	// Give the transmitter a chance to finish, and then to create an
	// interrupt when done
	sys->io_pic = INT_UARTTX;
}

void    txstr(const char *str) {
	const	char *ptr = str;
	while(*ptr)
		txchr(*ptr++);
}

void	txval(int val) {
	txstr("\r\n0x");
	for(int i=28; i>=0; i-=4) {
		int ch = ((val>>i)&0x0f)+'0';
		if (ch > '9')
			ch = ch - '0'+'A'-10;
		txchr(ch);
	}
}

void	txhex(int val) {
	for(int i=28; i>=0; i-=4) {
		int ch = ((val>>i)&0x0f)+'0';
		if (ch > '9')
			ch = ch - '0'+'A'-10;
		txchr(ch);
	}
	txstr("\r\n");
}

