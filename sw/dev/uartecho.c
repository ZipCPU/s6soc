////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	uartecho.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To simply test if both parts of the UART work.  All this file
//		does is attempt to receive a character from the UART, and if
//	successful, print it back out the UART.
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
// with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
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

void	entry(void) {
	SYSPIC = INT_CLEARPIC;

	while(1) {
		int	ch;

		SYSPIC = INT_UARTRX;
		while((SYSPIC & INT_UARTRX)==0)
			;
		ch = _uart & 0x0ff;

		SYSPIC = INT_UARTTX;
		while((SYSPIC & INT_UARTTX)==0)
			;
		_uart = ch;
	}
}
