////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	display.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	A device driver for the SPI controlled display.  The primary
//		purpose of this "device" is to output SPI instructions, though,
//	not to handle screen position or to optimize data set to the display.
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
#include "display.h"

static void	dispwait(void) {
	// Slow us down to the speed the display can handle
	// We want to delay about 0x800 clocks.
	// Three instructions per loop,
	//	LBL:
	//		NOOP
	//		ADD -1,CTR
	//		BNZ LBL
	//	40 clocks per instruction, as required by the SPI flash memory
	int i = 18;
	do {
		asm("noop");
	} while(i-->0);
}

void	dispchar(char ch) {
	if (!ch) // Don't display null characters
		return;
	IOSPACE	*sys = (IOSPACE *)IOADDR;
	// Send the character
	for(int i=0; i<8; i++) {
		int gpiov = GPOCLRV(GPO_MOSI|GPO_SCK|GPO_SS);
		if (ch&0x80)
			gpiov |= GPOSETV(GPO_MOSI);
		sys->io_gpio = gpiov;
		dispwait();
		sys->io_gpio = GPOSETV(GPO_SCK);
		dispwait();
		ch<<=1;
	}

	// Turn off the clock
	sys->io_gpio = GPOCLRV(GPO_SCK);
	dispwait();
	// Then the port
	sys->io_gpio = GPOSETV(GPO_SS);
	dispwait();
}

#ifdef	ZIPOS
#include "../zipos/ktraps.h"
#include "../zipos/kfildes.h"
void	display_task(void) {
	while(1) {
		int	ch;
		read(FILENO_STDIN, &ch, 1);
		dispchar(ch);
	}
}
#endif
