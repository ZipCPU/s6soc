////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	doorbell.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	This is the user program, or perhaps more appropriately
//		user program(s), associated with running the ZipOS on the
//	CMod-S6.  To run within the ZipOS, a user program must implement
//	two functions: kntasks() and kinit(TASKP *).  The first one is simple.
//	it simply returns the number of tasks the kernel needs to allocate 
//	space for.  The second routine needs to allocate space for each task,
//	set up any file descriptors associated with (each) task, and identify
//	the entry point of each task.  These are the only two routines
//	associated with user tasks called from kernel space.  Examples of each
//	are found within here.
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
#include "zipsys.h"
#include "board.h"
#include "ksched.h"
#include "kfildes.h"
#include "taskp.h"
#include "syspipe.h"
#include "ktraps.h"
#include "errno.h"
#include "swint.h"
#include "txfns.h"

#include "../dev/display.h"
#include "../dev/rtcsim.h"
#include "../dev/keypad.h"

typedef	unsigned	size_t;

size_t	strlen(const char *);
char *strcat(char *, const char *);
char *strcpy(char *, const char *);

/* Our system will need some pipes to handle ... life.  How about these:
 *
 *	rxpipe	- read()s from this pipe read from the UART
 *			Interrupt fed
 *	txpipe	- write()s to this pipe write to the UART
 *			Interrupt consumed
 *	keypipe	- read()s from this pipe return values read by the keypad
 *	lcdpipe	- write()s to this pipe write to the LCD display SPI port
 *	pwmpipe	- write()s to this pipe will send values to the audio port
 *			Interrupt consumed
 *
 * These pipes are allocated within the kernel setup function, ksetup().
 */

/* We'll need some tasks as well:
 *	User command task
 *		Handles user interaction
 *			Reads from pipe--either the keypad or the UARTRX pipe
 *			(Might be two such tasks in the system, one for each.)
 *		Sets clock upon request
 *		Reads from a pipe (rxpipe or keypipe), Writes to the txpipe pipe
 *	Doorbell task
 *		Maintains system time on the clock	: TIME: HH:MM:SS
 *		Maintains system status on display	: Light is (dis/en)abled
 *		Transitions when the doorbell is rung to: (fixed time line)
 *							: DOORBELL!!
 *		When the doorbell is clear, returns to the original task.
 *		---
 *		Waits on events, writes to the lcdpipe and pwmpipe.
 *		Reads from a command pipe, so that it can handle any user menu's
 *			Command pipe.  This, though, is tricky.  It requires
 *			a task that can be interrupted by either an event or a
 *			pipe.  Blocking is going to be more tricky ...
 *	Keypad task
 *		Normally, you might think this should be an interrupt task.
 *		But, it needs state in order to have timeouts and to debounce
 *		the input pin.  So ... let's leave this as a task.
 *		---
 *		Waits on events(keypad/timer), writes to the keypipe
 *	Display task
 *		The display does *not* need to be written to at an interrupt
 *		level.  It really needs to be written to at a task level, so
 *		let's make a display task.
 *		---
 *		Reads from the lcdpipe
 *	Real-time Clock Task
 *		Gets called once per second to update the real-time clock
 *		and to post those updates as an event to other tasks that might
 *		be interested in it.
 *		---
 *		Waits on system tasks, uses two semaphores
 */


/*
 * Read the keypad, write the results to an output pipe
 */
// #define	KEYPAD_TASK	keypad_task_id
/*
 * Read from the keypad, and set up a series of menu screens on the Display,
 * so that we can:
 *
 *	1. Set time
 * 	2. Set dawn
 *	3. Set dusk
 */
#define	MENU_TASK	menu_task_id
/*
 * Maintain a realtime clock
 */
#define	RTCCLOCK_TASK	rtccclock_task_id
/*
 * Read from an incoming pipe, write results to the SPI port controlling the
 * display.
 */
#define	DISPLAY_TASK	display_task_id

/*
 * Wait for a button press, and then based upon the clock set a light
 */
#define	DOORBELL_TASK	doorbell_task_id


/*
 * Just print Hello World every 15 seconds or so.  This is really a test of the
 * write() and txpipe infrastructure, but not really a valid part of the task.
 *
 */
// #define	HELLO_TASK	hello_task_id

#define	LAST_TASK	last_task_id

typedef	enum	{
#ifdef	RTCCLOCK_TASK
	RTCCLOCK_TASK,
#endif
#ifdef	DOORBELL_TASK
#ifdef	DISPLAY_TASK
	DOORBELL_TASK, DISPLAY_TASK,
#endif
#endif
//#ifdef	KEYPAD_TASK
	//KEYPAD_TASK,
//#endif
#ifdef	MENU_TASK
	MENU_TASK,
#endif
#ifdef	HELLO_TASK
	HELLO_TASK,
#endif
	LAST_TASK
} TASKNAME;


void	rtctask(void),
	doorbell_task(void),
	display_task(void),
	keypad_task(void),
	menu_task(void),
	hello_task(void);
	// idle_task ... is accomplished within the kernel
extern	void	restore_context(int *), save_context(int *);
extern	SYSPIPE	*rxpipe, *txpipe, *pwmpipe, *lcdpipe;
SYSPIPE *midpipe;
extern	KDEVICE *pipedev;

int	kntasks(void) {
	return LAST_TASK;
} void	kinit(TASKP *tasklist) {
#ifdef	RTCCLOCK_TASK
	// Stack = 36 (rtctask) + 4(rtcdatenext)
	tasklist[RTCCLOCK_TASK]    = new_task(64, rtctask);
#endif

#ifdef	DOORBELL_TASK
#ifdef	DISPLAY_TASK
	// Stack = 36 + 36 (uread/write) + 24(memcpy) + 32(uarthex)+8(uartchr)
	tasklist[DOORBELL_TASK]    = new_task(256, doorbell_task);
//	tasklist[DOORBELL_TASK]->fd[FILENO_STDOUT]= kopen((int)lcdpipe,pipedev);
	tasklist[DOORBELL_TASK]->fd[FILENO_STDERR]= kopen((int)txpipe, pipedev);
	tasklist[DOORBELL_TASK]->fd[FILENO_AUX] = kopen((int)pwmpipe,  pipedev);

	// Stack = 16 + 36(uread/write) + 24(memcpy)
	tasklist[DISPLAY_TASK] = new_task(128, display_task);
	tasklist[DISPLAY_TASK]->fd[FILENO_STDIN] = kopen((int)lcdpipe,pipedev);
#endif
#endif


#ifdef	KEYPAD_TASK
	// Stack = 28 + 36(uwrite) + 24(memcpy)		= 88 bytes
	tasklist[KEYPAD_TASK] = new_task(128, keypad_task);
	tasklist[KEYPAD_TASK]->fd[FILENO_STDOUT] = kopen((int)keypipe,pipedev);
#endif
#ifdef	MENU_TASK
	// Stack = 76   + 48(showbell/shownow)
	//		+ 36(uwrite)
	//		+  8(menu_readkey)
	//		+ 24(memcpy)
	//		+100(time_menu/dawn_menu/dusk_menu)
	//
	tasklist[MENU_TASK] = new_task(512, menu_task);
	// tasklist[MENU_TASK]->fd[FILENO_STDIN] = kopen((int)keypipe,pipedev);
	tasklist[MENU_TASK]->fd[FILENO_STDOUT]= kopen((int)lcdpipe,pipedev);
	tasklist[MENU_TASK]->fd[FILENO_STDERR]= kopen((int)txpipe, pipedev);
#endif

#ifdef	HELLO_TASK
	tasklist[HELLO_TASK] = new_task(512, hello_task);
	tasklist[HELLO_TASK]->fd[FILENO_STDOUT]= kopen((int)txpipe,pipedev);
#endif
}

// #define	HALF_HOUR_S	1800	// Seconds per half hour
// #define	HALF_HOUR_S	180	// Seconds per three minutes--for test
#define	HALF_HOUR_S	30	// 3 Mins is to long, here's 3 seconds

#ifdef	MENU_TASK
unsigned	dawn = 0x060000, dusk = 0x180000;
#else
const unsigned	dawn = 0x060000, dusk = 0x180000;
#endif

const char	basemsg[]   = "\e[jTime: xx:xx:xx\e[1;0H ";
const	char	nighttime[] = "Night time";
const	char	daylight[]  = "Daylight!";
const	char	dbellstr[]  = "Doorbell!";
void	shownow(unsigned now) {
	char	dmsg[40];
	strcpy(dmsg, basemsg);

	dmsg[ 9] = ((now>>20)&0x0f)+'0';
	dmsg[10] = ((now>>16)&0x0f)+'0';
	//
	dmsg[12] = ((now>>12)&0x0f)+'0';
	dmsg[13] = ((now>> 8)&0x0f)+'0';
	//
	dmsg[15] = ((now>> 4)&0x0f)+'0';
	dmsg[16] = ((now    )&0x0f)+'0';

	if ((now < dawn)||(now > dusk)) {
		strcat(dmsg, nighttime);
	} else {
		strcat(dmsg, daylight);
	} write(FILENO_STDOUT, dmsg, strlen(dmsg));
}

void	showbell(unsigned now) {	// Uses 10 stack slots + 8 for write()
	char	dmsg[40];

	strcpy(dmsg, basemsg);

	dmsg[ 9] = ((now>>20)&0x0f)+'0';
	dmsg[10] = ((now>>16)&0x0f)+'0';
	//
	dmsg[12] = ((now>>12)&0x0f)+'0';
	dmsg[13] = ((now>> 8)&0x0f)+'0';
	//
	dmsg[15] = ((now>> 4)&0x0f)+'0';
	dmsg[16] = ((now    )&0x0f)+'0';

	strcat(dmsg, dbellstr);
	write(FILENO_STDOUT, dmsg, strlen(dmsg));
}

void	uartchr(char v) {
	if (write(FILENO_STDERR, &v, 1) != 1)
		write(FILENO_STDERR, "APPLE-PANIC\r\n", 13);
}

void	uartstr(const char *str) {
	int	cnt;
	cnt = strlen(str);
	if (cnt != write(FILENO_STDERR, str, cnt))
		write(FILENO_STDERR, "PIPE-PANIC\r\n", 12);
}

void	uarthex(int num) {
	for(int ds=28; ds>=0; ds-=4) {
		int ch;
		ch = (num>>ds)&0x0f;
		if (ch >= 10)
			ch = 'A'+ch-10;
		else
			ch += '0';
		uartchr(ch);
	} uartstr("\r\n");
}

#ifdef DOORBELL_TASK
#include "../dev/samples.c"

void	belllight(unsigned now) {
	if ((now < dawn)||(now > dusk))
		_sys->io_spio = 0x088; // Turn our light on
	else
		_sys->io_spio = 0x80; // Turn light off
}

void	doorbell_task(void) {
	// Controls LED 0x08

	// Start by initializing the display to GT Gisselquist\nTechnology
	// write(KFD_STDOUT, disp_build_backslash,sizeof(disp_build_backslash));
	// write(KFD_STDOUT, disp_build_gtlogo, sizeof(disp_build_gtlogo));
	// write(KFD_STDOUT, disp_reset_data, sizeof(disp_reset_data));
	// write(KFD_STDOUT, disp_gtech_data, sizeof(disp_gtech_data));

	while(1) {
		int	event;
		// Initial state: doorbell is not ringing.  In this state, we
		// can wait forever for an event
		_sys->io_spio = 0x080; // Turn our light off
		event = wait(INT_BUTTON|SWINT_PPS,-1);

#ifndef	MENU_TASK
		unsigned when = rtcclock;
		if (event & INT_BUTTON)
			showbell(when);
		else if (event & SWINT_PPS)
			shownow(when);
#else
		if (event & INT_BUTTON)
			post(SWINT_DOORBELL);
#endif

		while(event & INT_BUTTON) {
			// Next state, the button has been pressed, the
			// doorbell is ringing

			// Seconds records the number of seconds since the
			// button was last pressed.
			int	seconds = 0;

			// Check time: should we turn our light on or not?
			belllight((volatile unsigned)rtcclock);
			const short *sptr = sound_data;
			while(sptr < &sound_data[NSAMPLE_WORDS]) {
				int	len = &sound_data[NSAMPLE_WORDS]-sptr;
				if (len > 512)
					len = 512;

				// We will stall here, if the audio FIFO is full
				write(FILENO_AUX, sptr,
					sizeof(sound_data[0])*len);
				sptr += len;

				// If the user presses the button more than
				// once, we start the sound over as well as
				// our light counter.
				event = wait(INT_BUTTON|SWINT_PPS, 0);
				if (event&INT_BUTTON) {
					if (sptr > &sound_data[1024]) {
						sptr = sound_data;
						seconds = 0;
#ifndef	MENU_TASK
						when = (volatile unsigned)rtcclock;
						showbell(when);
#else
						post(SWINT_DOORBELL);
#endif
					}
				} else if (event&SWINT_PPS) {
					seconds++;
					belllight((volatile unsigned)rtcclock);
#ifndef	MENU_TASK
					showbell(when);
#endif
				}
			}

			// Next state: the doorbell is no longer ringing, but
			// we have yet to return to normal--the light is still
			// on.
			while((seconds < HALF_HOUR_S)&&
				(((event=wait(INT_BUTTON|SWINT_PPS,-1))&INT_BUTTON)==0)) {
				seconds++;
				belllight((volatile unsigned)rtcclock);
#ifndef	MENU_TASK
				showbell(when);
#endif
			}
			if (event&INT_BUTTON) {
#ifndef	MENU_TASK
				when = (volatile unsigned)rtcclock;
				showbell(when);
#endif
			}
		}
	}
}
#endif

#ifdef	MENU_TASK
const char	menustr[] = "\e[1;0H     :    ";

void	entered_menu_str(char *str, unsigned now,int pos) {
	//
	// Set current time
	//   xx:xx:xx
	//
	strcpy(str, menustr);
	if (pos>0) {
		int ch = ((now >> 20)&0x0f)+'0';
		str[9] = ch;

		if (pos > 1) {
			ch = ((now >> 16)&0x0f)+'0';
			str[10] = ch;

		if (pos > 2) {
			ch = ((now >> 12)&0x0f)+'0';
			str[12] = ch;

		if (pos > 3) {
			int ch = ((now >> 8)&0x0f)+'0';
			str[13] = ch;

		if (pos > 4) {
			ch = ((now >> 4)&0x0f)+'0';
			str[15] = ch;
			str[14] = ':';

			if (pos > 5)
				ch = (now&0x0f)+'0';
			else
				ch = 'x';
			str[16] = ch;
	}}}}} str[17] = '\0';
}

const char	timmenu[] = "\e[jSet current time:";

void	show_time_menu(unsigned when, int posn) {
	char	dmsg[64];
	strcpy(dmsg, timmenu);
	entered_menu_str(&dmsg[20], when, posn);
	write(FILENO_STDOUT, dmsg, strlen(dmsg));
}

const char	dawnmenu[] = "\e[jSet sunrise: ";
void	show_dawn_menu(unsigned when, int posn) {
	char	dmsg[64];
	strcpy(dmsg, dawnmenu);
	entered_menu_str(&dmsg[16], when, posn);
	write(FILENO_STDOUT, dmsg, strlen(dmsg));
}

const char	duskmenu[] = "\e[;Set sunset: ";
void	show_dusk_menu(unsigned when, int posn) {
	char	dmsg[64];
	entered_menu_str(&dmsg[15], when, posn);
	write(FILENO_STDOUT, dmsg, strlen(dmsg));
}

int	menu_readkey(void) {
	int	key;
	wait(0,3);
	key = keypadread();
	keypad_wait_for_release();
	clear(INT_KEYPAD,0);
	return key;
}

void	time_menu(void) {
	int	timeout = 60;
	unsigned newclock = 0;
	for(int p=0; p<6; p++) {
		int	key, event;
		show_time_menu(newclock, p);
		do {
			event = wait(SWINT_PPS|INT_KEYPAD,-1);
			if (event&SWINT_PPS) {
				timeout--;
				if (timeout == 0)
					return;
			} if (event&INT_KEYPAD) {
				timeout = 60;
				key = menu_readkey();
				if ((key >= 0)&&(key < 10)) {
					int	sh;
					sh = (5-p)*4;
					newclock &= ~(0x0f<<sh);
					newclock |= (key<<sh);
				} else if (key == 12) {
					if (p>=0)
						p--;
				} else {
					if (p > 4)
						break;
					else
						return;
				}
			} txstr("Clk: "); txhex(newclock);
		} while(0==(event&INT_KEYPAD));
	}

	// Here's the trick here: without semaphores, we can't prevent a 
	// race condition on the clock.  It may be that the clock simulator
	// has read the clock value and is in the process of updating it, only
	// to have our task swapped in.  The risk here is that the RTC simulator
	// will write the updated value after we update our value here.  If it
	// does that, it will then set the SWINT_PPS interrupt.  So let's clear
	// this interrupt and then set our clock.  If the interrupt then
	// takes place in short order, we'll set the clock again.  That way,
	// if the RTC device was in the process of setting the clock, and then
	// sets it, we can adjust it again.
	//
	// Of course ... this won't work if it takes the clock longer than 
	// a millisecond to finish setting the clock ... but this is such a
	// rare race condition, and the consequences so minor, that this will
	// probably continue to work for now.
	clear(SWINT_PPS,0);
	rtcclock = newclock;
	if (wait(SWINT_PPS, 1))
		rtcclock = newclock;
}

void	dawn_menu(void) {
	int	timeout = 60;
	unsigned newdawn = 0;
	for(int p=0; p<6; p++) {
		int	key, event;
		show_dawn_menu(newdawn, p);
		do {
			event = wait(SWINT_PPS|INT_KEYPAD,-1);
			if (event&SWINT_PPS) {
				timeout--;
				if (timeout == 0)
					return;
			} if (event&INT_KEYPAD) {
				timeout = 60;
				key = menu_readkey();
				if ((key >= 0)&&(key < 10)) {
					int	sh = (5-p)*4;
					newdawn &= ~(0x0f<<sh);
					newdawn |= key<<sh;
				} else if (key == 12) {
					if (p>=0)
						p--;
				} else {
					if (p > 4)
						break;
					else
						return;
				}
			}
		} while(0 == (event&INT_KEYPAD));
	} dawn = newdawn;
}

void	dusk_menu(void) {
	int	timeout = 60;
	unsigned newdusk = 0;
	for(int p=0; p<6; p++) {
		int	key, event;
		show_dusk_menu(newdusk, p);
		do {
			event = wait(SWINT_PPS|INT_KEYPAD,-1);
			if (event&SWINT_PPS) {
				timeout--;
				if (timeout == 0)
					return;
			} if (event&INT_KEYPAD) {
				key = menu_readkey();
				if ((key >= 0)&&(key < 10)) {
					int	sh = (5-p)*4;
					newdusk &= ~(0x0f<<sh);
					newdusk |= key<<sh;
				} else if (key == 12) {
					if (p>=0)
						p--;
				} else {
					if (p > 4)
						break;
					else
						return;
				}
			}
		} while(0 == (event&INT_KEYPAD));
	} dusk = newdusk;
}

const char	unknownstr[] = "\e[jUnknown Cmd Key\e[1;0HA/Tm B/Dwn C/Dsk";
void	unknown_menu(void) {
	//	0123456789ABCDEF
	//	Unknown Cmd Key
	//	A/Tm B/Dwn C/Dsk
	write(FILENO_STDOUT, unknownstr, strlen(unknownstr));
}
void	menu_task(void) {
	// Controls LED 0x08

	// Start by initializing the display to GT Gisselquist\nTechnology
	// write(KFD_STDOUT, disp_build_backslash,sizeof(disp_build_backslash));
	// write(KFD_STDOUT, disp_build_gtlogo, sizeof(disp_build_gtlogo));
	// write(KFD_STDOUT, disp_reset_data, sizeof(disp_reset_data));
	// write(KFD_STDOUT, disp_gtech_data, sizeof(disp_gtech_data));
	unsigned belltime = 0, when;

	when = (volatile unsigned)rtcclock;
	while(1) {
		int	event;
		// Initial state: doorbell is not ringing.  In this state, we
		// can wait forever for an event
		event = wait(SWINT_DOORBELL|SWINT_PPS|INT_KEYPAD,-1);
		if (event & SWINT_DOORBELL) {
			showbell(when);
			belltime = time();
		} else if (event & SWINT_PPS) {
			unsigned	now = time();
			if ((now-belltime)<HALF_HOUR_S)
				showbell(when);
			else {
				when = (volatile unsigned)rtcclock;
				shownow(when);
			}
		}

		if (event & INT_KEYPAD) {
			int	key;
			key = menu_readkey();
			switch(key) {
				case 10: time_menu();
					when = (volatile unsigned)rtcclock;
					break;
				case 11: dawn_menu(); break;
				case 12: dusk_menu(); break;
				default:
					unknown_menu();
					wait(0,3000);
			} clear(INT_KEYPAD,0);
		}
	}
}
#endif


#ifdef	HELLO_TASK
static const char *hello_string = "Hello, World!\r\n";
void	hello_task(void) {
	while(1) {
		for(int i=0; i<15; i++)
			wait(SWINT_CLOCK, -1);
		write(FILENO_STDOUT, hello_string, strlen(hello_string));
	}
}
#endif
