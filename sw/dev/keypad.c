////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	keypad.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	A device driver (task) for the 16 character keypad.
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
#ifdef ZIPOS
#include "../zipos/ktraps.h"
#include "../zipos/kfildes.h"
#endif

static const char keymap[] = {
	0x01,0x02,0x03,0x0a,
	0x04,0x05,0x06,0x0b,
	0x07,0x08,0x09,0x0c,
	0x00,0x0f,0x0e,0x0d
};

char	keypadread(void) {
	int	row, col, key;
	IOSPACE	*sys = (IOSPACE *)IOADDR;

	row = sys->io_spio & 0x0f00;
	if (row != 0x0f00) {
		// If a button is still pressed .. 
		//
		// Check columns one and two to see if they were responsible
		// for the button
		sys->io_spio = 0x0cf00;
		// Get the result
		col = sys->io_spio;
		if ((col&0x0f00)!=0x0f00) {
			// Column one or two is pressed
			sys->io_spio = 0x0ef00;
			col = sys->io_spio;
			if ((col&0x0f00)!=0x0f00)
				col = 0;
			else
				col = 1;
		} else {
			// Must be column three or four
			sys->io_spio = 0x7f00;
			col = sys->io_spio;
			if ((col & 0x0f00)!= 0x0f00) // Column 4
				col = 3;
			else
				col = 2;
		} sys->io_spio = 0x0f00; // Reset column pins to zero
		if (row == (int)(sys->io_spio & 0x0f00)) {
			// The key didn't change, so we might have something
			row >>= 8;
			row &= 0x0f;
			row ^= 0x0f;
			
			// Found the pin
			if (row == 1)
				row=0;
			else if (row == 2)
				row = 1;
			else if (row == 4)
				row = 2;
			else if (row == 8)
				row = 3;
			else
				// Two or more buttons were pressed
				// -- declare an error
				row = -1;

			if (row>=0) {
				key = (row |(col<<2));
				key = keymap[key];
			} else key = -1;
		} else key = -1;
	} else key = -1;

	if (sys->io_pic & INT_ENABLE)
		sys->io_pic = INT_ENABLE|INT_KEYPAD;
	else sys->io_pic = INT_KEYPAD;

	return key;
}

void	keypad_wait_for_release(void) {
	IOSPACE *sys = (IOSPACE *)IOADDR;
	sys->io_spio = 0x0f00;
	while((sys->io_spio & 0x0f00)!=0x0f00)
#ifdef	ZIPOS
		wait(0,2);
#else
		;
#endif
}

#ifdef ZIPOS
void keypad_task(void) {
	clear(INT_KEYPAD, 0);
	while(1) {
		int	key;
		wait(INT_KEYPAD,-1); // Automatically clears
		key = keypadread();
		write(FILENO_STDOUT, &key, 1);
		// Prepare for the next key
		clear(INT_KEYPAD, 0);
	}
}
#endif
