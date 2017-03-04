////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	doorbell2.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	A modification to the original doorbell.c program.
//		seconds.  Listening to that test is ... getting old.
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

#include "samples.c"

void	zip_halt(void);

void	build_dpymsg(char *msg, unsigned clkval);
void	build_uartmsg(char *msg, unsigned clkval);
void	showval(int val);
void	txval(int val);

void entry(void) {
	register IOSPACE	*sys = (IOSPACE *)0x0100;
	char	dpymsg[64], *dpyptr;
	char	uartmsg[160], *uartptr;
	int	newmsgtime = 0, leastmsgtime = -1, lstmsgtime = 0;

	dpymsg[0] = 0;
	dpyptr = dpymsg;

	uartmsg[0] = 0;
	build_uartmsg(uartmsg, 0);
	uartptr = uartmsg;

	sys->io_timb = 0;
	sys->io_pic = 0x07fffffff; // Acknowledge and turn off all interrupts

	sys->io_spio = 0x0f4;
	newmsgtime = sys->io_tima;
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
		sys->io_tima = TM_ONE_SECOND | TM_REPEAT;

		// We start by waiting for a doorbell
		while(((pic=sys->io_pic) & INT_BUTTON)==0) {
			if (uartmsg[10] == 0) {
				sys->io_spio = 0x0fe;
				zip_halt();
			}
			if (pic & INT_TIMA) {// top of second
				sys->io_pic = INT_TIMA;
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
					newmsgtime = sys->io_tima;
					lstmsgtime = -1;
					leastmsgtime = -1;
				}
			}
			/*
			if (uartmsg[10] == 0) {
				sys->io_spio = 0x0fc;
				zip_halt();
			}
			*/
			if (*uartptr) {
				if (pic & INT_UARTTX) {
					sys->io_uart = *uartptr++;
					sys->io_spio = 0x22;
					sys->io_pic = INT_UARTTX;
					if (uartptr > &uartmsg[13]) {
						sys->io_spio = 0x0fd;
						zip_halt();
					}

					if (lstmsgtime != -1) {
						int tmp;
						tmp = (lstmsgtime-sys->io_tima);
						if ((leastmsgtime<0)||(tmp<leastmsgtime))
							leastmsgtime = tmp;
					} lstmsgtime = sys->io_tima;
				}
			} else {
				sys->io_spio = 0x20;
				/*
				if (newmsgtime != 0) {
					int thistime = sys->io_tima;
					thistime = newmsgtime - thistime;
					showval(thistime);
					txval(thistime);
					txval(leastmsgtime);
					txval(lstmsgtime);
					zip_halt();
					newmsgtime = 0;
				}
				for(int i=0; i<12; i++)
					if (uartmsg[i] == 0) {
						sys->io_spio = i+0xf0;
						zip_halt();
					}
				*/
			}
			if (*dpyptr) {
				// This will take a long time.  It should be an
				// interruptable task ... but, sigh, we're not
				// there yet.
				dispchar(*dpyptr++);
				sys->io_spio = 0x44;
			} else {
				sys->io_spio = 0x40;
			} // sys->io_pic = (pic & (INT_TIMA|INT_UARTTX));
		}

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
			unsigned	this_sample;
			do {
				pic = sys->io_pic;
				if (pic & INT_TIMA) {
					sys->io_pic = INT_TIMA;
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
					sys->io_spio = 0x44;
				} else
					sys->io_spio = 0x40;
			} while((pic & INT_AUDIO)==0);
			this_sample = (*sptr++) & 0x0ffff;
			sys->io_pwm_audio = this_sample;
			// Now, turn off the audio interrupt since it doesn't
			// reset itself ...
			sys->io_pic = INT_AUDIO;
		} sys->io_pic = INT_BUTTON;

		// Now we wait for the end of our 30 second window
		sys->io_spio = 0x0f8;
		sys->io_pwm_audio = 0x018000; // Turn off the Audio device
		while(seconds < 30) {
			pic = sys->io_pic;
			if (pic & INT_TIMA) {
				sys->io_pic = INT_TIMA;
				seconds++;
				rtcclock = rtcnext(rtcclock);
			} if (pic & INT_BUTTON) {
				sys->io_pic = INT_BUTTON;
				seconds = 0;
			}
		} sys->io_pic = INT_BUTTON;
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

void	txch(char val) {
	register IOSPACE	*sys = (IOSPACE *)0x0100;
	unsigned v = (unsigned char)val;

	// To read whether or not the transmitter is ready, you must first
	// clear the interrupt bit.
	sys->io_pic = INT_UARTTX;
	for(int i=0; i<5000; i++)
		asm("noop");
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

void	txval(int val) {
	txch('\r');
	txch('\n');
	txch('0');
	txch('x');
	for(int i=28; i>=0; i-=4) {
		int ch = ((val>>i)&0x0f)+'0';
		if (ch > '9')
			ch = ch - '0'+'A'-10;
		txch(ch);
	}
}

// PPONP16P
// 00120O91
// 00120NM3
// 00120E91 = 1183377 ~= 91029 / char, at 0x208d 8333/baud, 83,330 per char
