////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	doorbell.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To test the PWM device by playing a doorbell sound every ten
//		seconds.
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

#include "samples.c"

const char	msg[] = "Doorbell!\r\n\r\n";

void entry(void) {
	register volatile IOSPACE *const sys = _sys;

	sys->io_watchdog = 0;
	sys->io_pic = 0x07fffffff; // Acknowledge and turn off all interrupts

	sys->io_spio = 0x0f4;
	sys->io_pwm_audio = 0x0110000;
	while(1) {
		int	seconds = 0, pic;
		const unsigned short	*ptr;
		const char	*mptr = msg;
		sys->io_timer = TM_ONE_SECOND | TM_REPEAT; // Ticks per second, 80M

		sys->io_spio = 0x0f0;
		ptr = (const unsigned short *)sound_data;
		sys->io_pwm_audio = 0x0310000;
		sys->io_spio = 0x0f1;
		while(ptr <(const unsigned short *)&sound_data[NSAMPLE_WORDS]) {
			unsigned	this_sample;
			sys->io_spio = 0x022;
			do {
				pic = sys->io_pic;
				if (pic & INT_TIMER)
					seconds++;
				if ((pic & INT_UARTTX)&&(*mptr))
					sys->io_uart = *mptr++;
				sys->io_pic = (pic & 0x07fff);
			} while((pic & INT_AUDIO)==0);
			this_sample = (*ptr++); // & 0x0ffff;
			sys->io_pwm_audio = this_sample;
			// Now, turn off the audio interrupt since it doesn't
			// reset itself ...
			sys->io_pic = INT_AUDIO;
		} if (ptr >= (const unsigned short *)&sound_data[NSAMPLE_WORDS])
			sys->io_spio = 0x044;

		sys->io_spio = 0x088;
		sys->io_pwm_audio = 0;
		while(seconds < 10) {
			pic = sys->io_pic;
			if (pic & INT_TIMER)
				seconds++;
			sys->io_pic = (pic & 0x07fff);
		}
		sys->io_spio = 0x0ff;
	}
}

