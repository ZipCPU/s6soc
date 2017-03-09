////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	doorbell2.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	A modification to the original doorbell.c program that played
//		a doorbell sound every ten seconds.  Listening to that test is
//	... getting old.
//
//	Let's let this one do the following:
//		1. Display the time on the display (it will be impossible to
//			change the time, sadly, but we can at least display it.)
//		2. On button press ...
//			- Play the doorbell sound
//			- Display "Doorbell!\n" on the Display, clearing the
//				time
//			- Send "Doorbell\n"  to the UART, and then keeping the
//				UART silent for 30 seconds.
//		4. Send the time to the UART as well, but only once a minute.
//			(and that if the Doorbell hasn't been rung in the last
//			30 seconds ...)
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
#include "rtcsim.h"
#include "display.h"
#include "string.h"
#include "txfns.h"

#include "samples.c"

void	zip_halt(void);

void	build_dpymsg(char *msg, unsigned clkval);
void	build_uartmsg(char *msg, unsigned clkval);
void	showval(int val);

#define	CLEAR_WATCHDOG	_sys->io_watchdog = 0
#define	TOUCH_WATCHDOG	_sys->io_watchdog = 500000

void entry(void) {
	register volatile IOSPACE *const sys = _sys;
	char	dpymsg[64], *dpyptr;
	char	uartmsg[160], *uartptr;
	int	newmsgtime = 0, leastmsgtime = -1, lstmsgtime = 0;

	CLEAR_WATCHDOG;

	txstr("\r\nREBOOT!\r\n");
	txstr("Scope Control = "); txval(_scope->s_ctrl);
	if (_scope->s_ctrl & WBSCOPE_STOPPED) {
		int	ln = WBSCOPE_LENGTH(_scope->s_ctrl);
		for(int i=0; i<ln; i++)
			txval(_scope->s_data);
		txval((int)_sys->io_buserr);
		txstr("\r\n\r\nEndScope\r\n");
	} else {
		txstr("\r\n");
		_scope->s_ctrl = 20;
	}

	dpymsg[0] = 0;
	dpyptr = dpymsg;

	uartmsg[0] = 0;
	build_uartmsg(uartmsg, 0);
	uartptr = uartmsg;

	sys->io_watchdog = 0;
	sys->io_pic = INT_CLEARPIC; // Acknowledge and turn off all interrupts

	sys->io_spio = 0x0f4;
	newmsgtime = sys->io_timer;
	leastmsgtime = -1;
	lstmsgtime = newmsgtime;
	while(1) {
		int	seconds, pic;
		const short	*sptr;

		// LED's off ... nothing to report
		sys->io_spio = 0x0f0;

		// Turn the audio off (initially)
		sys->io_pwm_audio = 0x0018000;

		// Set for one ticks per second, 80M clocks per tick
		sys->io_timer = TM_ONE_SECOND | TM_REPEAT;

		// We start by waiting for a doorbell
		while(((pic=sys->io_pic) & INT_BUTTON)==0) {
			TOUCH_WATCHDOG;

			if (pic & INT_TIMER) {// top of second
				sys->io_pic = INT_TIMER;
				rtcclock = rtcnext(rtcclock);

				// Turn all LED off (again)
				sys->io_spio = 0x0f0;
				if (*dpyptr == '\0') {
					// Build a message for the display
					build_dpymsg(dpymsg, rtcclock);
					dpyptr = dpymsg;
				}if(((rtcclock & 0x0ff)==0)&&(*uartptr=='\0')){
					build_uartmsg(uartmsg, rtcclock);
					uartptr = uartmsg;

					// Turn one LED on--top of minute
					sys->io_spio = 0x0f1;
					newmsgtime = sys->io_timer;
					lstmsgtime = -1;
					leastmsgtime = -1;
				}
			}

			if (*uartptr) {
				if (pic & INT_UARTTX) {
					sys->io_uart = *uartptr++;
					sys->io_spio = 0x22;
					sys->io_pic = INT_UARTTX;

					if (lstmsgtime != -1) {
						int tmp;
						tmp = (lstmsgtime-sys->io_timer);
						if ((leastmsgtime<0)||(tmp<leastmsgtime))
							leastmsgtime = tmp;
					} lstmsgtime = sys->io_timer;
				}
			} else {
				sys->io_spio = 0x20;
			}
			if (*dpyptr) {
				// This will take a long time.  It should be an
				// interruptable task ... but, sigh, we're not
				// there yet.
				dispchar(*dpyptr++);
				sys->io_spio = 0x44;
			} else {
				sys->io_spio = 0x40;
			} // sys->io_pic = (pic & (INT_TIMER|INT_UARTTX));
		}

		TOUCH_WATCHDOG;
		// DOORBELL!!!!!!
		// Set the Display message
		strcpy(dpymsg, "a[jDoorbell!");
		dpymsg[0] = 0x1b;
		dpyptr = dpymsg;
		// And the UART message / 18 characters
		uartptr = uartmsg;
		strcat(uartptr, "\r\nDoorbell!\r\n\r\n");
		uartptr = uartmsg;


		seconds = 0;
		sys->io_spio = 0x0ff; // All LED's on: we got one!
		sptr = sound_data;
		sys->io_pwm_audio = 0x0310000; // Turn on the audio
		while(sptr < &sound_data[NSAMPLE_WORDS]) {
			do {
				TOUCH_WATCHDOG;
				pic = sys->io_pic;
				if (pic & INT_TIMER) {
					sys->io_pic = INT_TIMER;
					seconds++;
					rtcclock = rtcnext(rtcclock);
				} if ((pic & INT_UARTTX)&&(*uartptr)) {
					sys->io_uart = *uartptr++;
					sys->io_pic = INT_UARTTX;
					sys->io_spio = 0x22;
				} else if (!*uartptr)
					sys->io_spio = 0x20;
				if (*dpyptr) {
				// This will take a long time.  It should be an
				// interruptable task ... but, sigh, we're not
				// there yet.
					dispchar(*dpyptr++);
					sys->io_spio = 0x45;
				} else
					sys->io_spio = 0x40;
			} while((pic & INT_AUDIO)==0);
			sys->io_pwm_audio = (*sptr++) & 0x0ffff;
			// Now, turn off the audio interrupt since it doesn't
			// reset itself ...
			sys->io_pic = INT_AUDIO;
		} sys->io_pic = INT_BUTTON;
		sys->io_spio = 0x10;

		TOUCH_WATCHDOG;
		// Now we wait for the end of our 30 second window
		sys->io_spio = 0x0f8;
		sys->io_pwm_audio = 0x018000; // Turn off the Audio device
		while(seconds < 30) {
			TOUCH_WATCHDOG;
			pic = sys->io_pic;
			if (pic & INT_TIMER) {
				sys->io_pic = INT_TIMER;
				seconds++;
				rtcclock = rtcnext(rtcclock);
			} if (pic & INT_BUTTON) {
				sys->io_pic = INT_BUTTON;
				seconds = 0;
			}
		} sys->io_pic = INT_BUTTON;
		TOUCH_WATCHDOG;
	}
}

void	build_dpymsg(char *msg, unsigned clk) {
	msg[0] = 0x1b;
	strcpy(++msg, "[jClock : ");
	msg += strlen(msg);

	if ((clk>>20)&0x0f)
		*msg++ |= (((clk>>20)&0x0f)+'0');
	else
		*msg++ |= ' ';
	*msg++ = (((clk>>16)&0x0f)+'0');
	*msg++ = ':';
	*msg++ = (((clk>>12)&0x0f)+'0');
	*msg++ = (((clk>> 8)&0x0f)+'0');
	*msg++ = ':';
	*msg++ = (((clk>> 4)&0x0f)+'0');
	*msg++ = (((clk    )&0x0f)+'0');
	*msg++ = 0;
}

void	build_uartmsg(char *msg, unsigned clk) {
	strcpy(msg, "Time: ");
	msg += strlen(msg);
	*msg++ = ((clk>>20)&0x03)+'0'; // Hrs
	*msg++ = ((clk>>16)&0x0f)+'0';
	*msg++ = ':';
	*msg++ = ((clk>>12)&0x0f)+'0'; // Mins
	*msg++ = ((clk>> 8)&0x0f)+'0';
	*msg++ = '\r';			// 11
	*msg++ = '\n';			// 12
	*msg++ = '\0';
	*msg++ = '\0';
}

void	showval(int val) {
	// Clear and home
	dispchar(0x1b);
	dispchar('[');
	dispchar('j');
	for(int i=28; i>=0; i-=4) {
		int ch = ((val>>i)&0x0f)+'0';
		if (ch > '9')
			ch = ch - '0'+'A'-10;
		dispchar(ch);
	}
}
