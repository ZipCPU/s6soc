////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	rtcsim.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	Since we lost the real-time clock on board due to space
//		constraints, this unit attempts to replace that functionality
//	in software.
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
#include "rtcsim.h"

unsigned	rtcclock, rtcalarm, rtcdate;
#ifdef	ZIPOS
// SEMAPHORE	SEMDATE, SEMCLOCK;
#endif

unsigned	rtcnext(unsigned now) {
	now = now+1;
	if ((now & 0x0f)>=0x0a) {
		now += 0x06; // Seconds
		if ((now & 0x0f0)> 0x050) {
			now += 0x0a0; // Increment minutes
			if ((now & 0x0f00)>= 0x0a00) {
				now += 0x0600;
				if ((now & 0x0f000)> 0x05000) {
					now += 0x0a000; // Increment hours
					if ((now & 0x0f0000)>= 0x0a0000)
						now += 0x060000;
					if (now >= 0x0240000)
						now = 0;
				}
			}
		}
	}
	return now;
}

static const char days_per_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

unsigned	rtcdatenext(unsigned today) {
	int dy, mo;

	dy = (today&0x0f)+((today>>4)&0x03)*10;
	mo = ((today>>8)&0x0f)+((today&0x0100)?10:0);
	dy = dy+1;
	if ((mo>12)||(dy > days_per_month[mo-1])) {
		if (mo == 2) {
			// If its a leap year
			//	If it is divisible by four
			//		But not a century year
			//			unless it's a fourth century yr
			int	tst;
			if ((today&0x0ff0000) == 0) {
				tst = (today >> 24) & 0x03f;
			} else {
				tst = (today >> 20) & 0x0ff;
			} tst &= 0x013;
			if ((tst==0)||(tst==0x012)) {
				// Divisible by four
				if (dy >= 29)
					dy = 1;
			} else dy = 1;
		} else dy = 1;
	} if (dy == 1) {
		// Set to the first day of the month
		today &= (~0x0ff);
		today |= 1;
		mo++;
		if (mo > 12) {
			// Set to the first month of the year
			today &= ~(0x0ff00);
			today |= 0x0100;
			// Increment the year
			today += 0x010000;
			if ((today & 0x0f0000)>=0x0a0000) {
				today += 0x060000;
				if ((today& 0x0f00000)>= 0x0a00000) {
					today += 0x0600000;
					if ((today & 0x0f000000)>= 0x0a000000) {
						today += 0x06000000;
						if ((today & 0x0f0000000)>=0x0a0000000)
							today += 0x060000000;
					}
				}
			}
		}
	} else {
		today += 1;
		if ((today & 0x0f)>=0x0a)
			today += 0x06;
	}
	return today;
}

#ifdef	ZIPOS
#include "board.h"
#include "../zipos/ktraps.h"
#include "../zipos/swint.h"

void rtctask(void) {
	// IOSPACE *sys = (IOSPACE *)IOADDR;
	rtcdate = 0x20170408;
	rtcclock = 0;
	while(1) {
		unsigned event = wait(SWINT_CLOCK,-1);
		if (event&SWINT_CLOCK) {
			unsigned v, nextevent = SWINT_PPS;
			semget(SEMCLOCK);
			rtcclock = v = rtcnext(rtcclock);
			semput(SEMCLOCK);
#ifdef	SWINT_ALARM
			if (v == rtcalarm)
				nextevent |= SWINT_ALARM;
#endif
			if (v == 0) {
				semget(SEMDATE);
				rtcdate = rtcdatenext(rtcdate);
				semput(SEMDATE);

#ifdef	SWINT_PPD
				nextevent |= SWINT_PPD;
#endif
			} post(nextevent);
		}
	}
}
#endif

