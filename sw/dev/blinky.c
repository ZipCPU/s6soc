////////////////////////////////////////////////////////////////////////////////
//
// Filename:	blinky.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To toggle/blink the LEDs in a fashion that will let us know
//		1) that the CPU is running, 2) that the LEDs are working, and
//	even better, 3) that the buttons are working.
//
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

void	zip_idle(void);

void entry(void) {
	const char	*msg = "Hello, World!\r\n", *ptr = msg;
	int	count = 0;

	_sys->io_spio = 0x0fa;
	_sys->io_timer = TM_REPEAT | (TM_ONE_SECOND/4);
	_sys->io_pic = INT_ENABLEV(INT_TIMER)|INT_CLEAR(INT_TIMER);

	do {
		zip_idle();
		int picv = _sys->io_pic;
		if (picv & INT_TIMER) {
			int ledv = _sys->io_spio;
			ledv <<= 1;
			ledv = ledv & 0x0f;
			if (ledv == 0)
				ledv = 1;
			_sys->io_spio = ledv | 0x0f0;

			if (*ptr)
				_sys->io_uart = (unsigned)*ptr++;
			if (count++ > 4*60) {
				count = 0;
				ptr = msg;
			}
		}

		_sys->io_pic = INT_ENABLEV(INT_TIMER)|INT_CLEAR(INT_TIMER);
	} while(1);
}

// PPONP16P
// 00120O91
// 00120NM3
// 00120E91 = 1183377 ~= 91029 / char, at 0x208d 8333/baud, 83,330 per char
