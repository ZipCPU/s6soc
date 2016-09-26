////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	kernel.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	If you are looking for a main() program associated with the
//		ZipOS, this is it.  This is the main program for the supervisor
//	task.  It handles interrupt processing, creating tasks, context swaps,
//	creating tasks, and ... just about everything else a kernel must handle.
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

extern	void	kpanic(void);
extern	void	raw_put_uart(int val);

unsigned int	nresets = 0;

extern int	kntasks(void);
extern void	kinit(TASKP *tasklist);
extern	void	restore_context(int *), save_context(int *);
SYSPIPE	*rxpipe, *txpipe, *keypipe, *lcdpipe, *pwmpipe, *cmdpipe;
KDEVICE	*pipedev, *txdev, *pwmdev;
void	*heap; //  = _top_of_heap; // Need to wait on startup to set this

#define	CONTEXT_LENGTH	80000	// 1ms
#define	TICKS_PER_SECOND	1000

void kwrite_audio(TASKP tsk, int dev, int *dst, int len);
void kwrite_txuart(TASKP tsk, int dev, int *dst, int len);
int	kpost(TASKP *task, unsigned events, int milliseconds);
TASKP	kschedule(int LAST_TASK, TASKP *tasklist, TASKP last);
extern TASKP	*ksetup(void);

int	LAST_TASK;

void	kernel_entry(void) {
	int	nheartbeats= 0, tickcount = 0, milliseconds=0, ticks = 0;
	int	audiostate = 0, buttonstate = 0;
	TASKP	*tasklist, current;
	int	*last_context;
	IOSPACE	*sys = (IOSPACE *)IOADDR;

	tasklist = ksetup();

	current = tasklist[0];
	restore_context(current->context);
	last_context = current->context;

	unsigned enableset = 
		INT_ENABLEV(INT_BUTTON)
		|INT_ENABLEV(INT_TIMA)
		// |INT_ENABLEV(INT_UARTRX)
		// |INT_ENABLEV(INT_UARTTX) // Needs to be turned on by driver
		// |INT_ENABLEV(INT_AUDIO // Needs to be turned on by driver)
		// |INT_ENABLEV(INT_GPIO)
		// |INT_ENABLEV(INT_TIMB);
		;
	// Then selectively turn some of them back on
	sys->io_pic = INT_ENABLE | enableset | 0x07fff;

	do {
		int need_resched = 0, context_has_been_saved, pic;
		nheartbeats++;

		zip_rtu();

		last_context = current->context;
		context_has_been_saved = 0;
		pic = sys->io_pic;

		if (pic & 0x8000) { // If there's an active interrupt
			// Interrupt processing
			sys->io_spio = 0x44;

			// First, turn off pending interrupts
			// Although we migt just write 0x7fff7fff to the
			// interrupt controller, how do we know another
			// interrupt hasn't taken place since we read it?
			// Thus we turn off the pending interrupts that we
			// know about.
			pic &= 0x7fff;
			// Acknowledge current ints, and turn off pending ints
			sys->io_pic = INT_DISABLEV(pic)|(INT_CLEAR(pic));
			if(pic&INT_TIMA) {
				if (++ticks >= TICKS_PER_SECOND) {//(pic & SYSINT_PPS)
					// Toggle the low order LED
					tickcount++;
					ticks = 0;
					sys->io_spio = ((sys->io_spio&1)^1)|0x010;
					pic |= SWINT_CLOCK;
				}
				if (buttonstate)
					buttonstate--;
				else
					enableset |= INT_ENABLEV(INT_BUTTON);
			}
			// 
			if (pic&INT_BUTTON) {
				// Need to turn the button interrupt off
				enableset &= ~(INT_ENABLEV(INT_BUTTON));
				if ((sys->io_spio&0x0f0)==0x030)
					kpanic();
				buttonstate = 3;
			}
			if (pic & INT_UARTRX) {
				int v = sys->io_uart;
	
				if ((v & (~0x7f))==0) {
					kpush_syspipe(rxpipe, v);

					// Local Echo
					if (pic & INT_UARTTX) {
						sys->io_uart = v;
						sys->io_pic = INT_UARTTX;
						pic &= ~INT_UARTTX;
					}
				}
			} if (pic & INT_UARTTX) {
				int	v;
				if (kpop_syspipe(txpipe, &v)==0) {
					enableset |= (INT_ENABLEV(INT_UARTTX));
					sys->io_uart= v;
					sys->io_pic = INT_UARTTX;
					// if (v == 'W')
						// sys->io_timb = 5;
						// 75k was writing the 'e'
				} else
					enableset &= ~(INT_ENABLEV(INT_UARTTX));
			} if (audiostate) {
				if (pic & INT_AUDIO) {
				int v;
				// States: 
				//	0 -- not in use
				//	1 -- First sample, buffer empty
				//		time to read a new sample
				//	2 -- second sample,  to read new
				//	3 -- Need to turn off
				if ((audiostate & 3)==2) {
					sys->io_pwm_audio = (audiostate>>2)&0x0ffff;
					audiostate = 1;
				} else if (kpop_syspipe(pwmpipe, &v)==0) {
					audiostate = (2|(v<<2))&0x03ffff;
					sys->io_pwm_audio = (v>>16)&0x0ffff;
				} else {
					audiostate = 0;
					// Turn the device off
					sys->io_pwm_audio = 0x10000;
					// Turn the interrupts off
					enableset &= ~(INT_ENABLEV(INT_AUDIO));
					sys->io_spio = 0x020;
				}

				// This particular interrupt cannot be cleared
				// until the port has been written to.  Hence,
				// now that we've written to the port, we clear
				// it now.  If it needs retriggering, the port
				// will retrigger itself -- despite being
				// cleared here.
				sys->io_pic = INT_AUDIO;
			}} else { // if (audiostate == 0)
				int	sample;
				if (kpop_syspipe(pwmpipe, &sample)==0) {
					audiostate = (2|(sample<<2))&0x03ffff;
					sys->io_pwm_audio = 0x310000 | ((sample>>16)&0x0ffff);
					enableset |= (INT_ENABLEV(INT_AUDIO));
					sys->io_spio = 0x022;
					sys->io_pic = INT_AUDIO;
				} // else sys->io_spio = 0x020;
			}
			milliseconds = kpost(tasklist, pic, milliseconds);

			// Restart interrupts
			enableset &= (~0x0ffff); // Keep the bottom bits off
			sys->io_pic = INT_ENABLE|enableset;
		} else {
			sys->io_pic = INT_ENABLE; // Make sure interrupts are on
			int	sample;

			// Check for the beginning of an audio pipe.  If the
			// interrupt is not enabled, we still might need to
			// enable it.
			if ((audiostate==0)&&(kpop_syspipe(pwmpipe, &sample)==0)) {
				audiostate = (2|(sample<<2))&0x03ffff;
				sys->io_pwm_audio = 0x310000 | ((sample>>16)&0x0ffff);
				sys->io_pic = INT_AUDIO;
				enableset |= (INT_ENABLEV(INT_AUDIO));
				sys->io_spio = 0x022;
			} // else sys->io_spio = 0x020;

			// Or the beginning of a transmit pipe.  
			if (pic & INT_UARTTX) {
				int	v;
				if (kpop_syspipe(txpipe, &v)==0) {
					enableset |= (INT_ENABLEV(INT_UARTTX));
					sys->io_uart = v;
					sys->io_pic = INT_UARTTX;
					// if (v == 'W')
						// sys->io_timb = 5;
				} else
					enableset &= ~(INT_ENABLEV(INT_UARTTX));
			}

			// What if someone left interrupts off?
			// This might happen as part of a wait trap call, such
			// as syspipe() accomplishes within uwrite_syspipe()
			// (We also might've just turned them off ... ooops)
			enableset &= (~0x0ffff); // Keep the bottom bits off
			sys->io_pic = INT_ENABLE | enableset;
		}
		sys->io_spio = 0x40;

		int zcc = zip_ucc();
		if (zcc & CC_TRAPBIT) {
			// sys->io_spio = 0x0ea;

			context_has_been_saved = 1;
			save_context(last_context);
			last_context[14] = zcc & (~CC_TRAPBIT);
			// Do trap handling
			switch(last_context[1]) {
			case TRAPID_WAIT:
				{ // The task wishes to wait on an interrupt
				int ilist, timeout;
				ilist = last_context[2];
				timeout= last_context[3];
				last_context[1] = ilist & current->pending;
				if (current->pending & ilist) {
					// Clear upon any read
					current->pending &= (~last_context[1]);
				} else {
					current->waitsig = ilist;
					if (timeout != 0) {
						current->state = SCHED_WAITING;
						need_resched = 1;
						if (timeout > 0) {
							current->timeout=milliseconds+timeout;
							current->waitsig |= SWINT_TIMEOUT;
						}
					}
				}} break;
			case TRAPID_CLEAR:
				{ unsigned timeout;
				// The task wishes to clear any pending
				// interrupts, in a likely attempt to create
				// them soon.
				last_context[1] = last_context[2] & current->pending;
				// Clear upon any read
				current->pending &= (~last_context[1]);
				timeout = (unsigned)last_context[2];
				if (timeout) {
					if ((int)timeout < 0)
						// Turn off any pending timeout
						current->pending &= (~SWINT_TIMEOUT);
					else
						// Otherwise, start a timeout
						// counter
						current->timeout = milliseconds+timeout;
				}} break;
			case TRAPID_POST:
				kpost(tasklist, last_context[2]&(~0x07fff),
						milliseconds);
				break;
			case TRAPID_YIELD:
				need_resched = 1;
				break;
			case TRAPID_READ:
				{
				KFILDES	*fd = NULL;
				if ((unsigned)last_context[2]
						< (unsigned)MAX_KFILDES)
					fd = current->fd[last_context[2]];
				if ((!fd)||(!fd->dev))
					last_context[1] = -EBADF;
				else
					fd->dev->read(current, fd->id,
					   (void *)last_context[3], last_context[4]);
				} break;
			case TRAPID_WRITE:
				{ KFILDES	*fd = NULL;
				if ((unsigned)last_context[2]
						< (unsigned)MAX_KFILDES)
					fd = current->fd[last_context[2]];
				else { kpanic(); zip_halt(); }
				if ((!fd)||(!fd->dev))
					last_context[1] = -EBADF;
				else {
					fd->dev->write(current, fd->id,
					   (void *)last_context[3], last_context[4]);
				}}
				break;
			case TRAPID_TIME:
				last_context[1] = tickcount;
				break;
			case TRAPID_MALLOC:
				last_context[1] = (int)sys_malloc(last_context[2]);
				break;
			case TRAPID_FREE:
				// Our current malloc cannot free
				// sys_free(last_context[2])
				break;
			case TRAPID_EXIT:
				current->state = SCHED_EXIT;
				need_resched = 1;
				kpanic();
				zip_halt();
				break;
			default:
				current->state = SCHED_ERR;
				need_resched = 1;
				kpanic();
				zip_halt();
				break;
			}

			restore_context(last_context);
		} else if (zcc & (CC_BUSERR|CC_DIVERR|CC_FPUERR|CC_ILL)) {
			current->state = SCHED_ERR;
			// current->errno = -EBUS;
			current->errno = (int)sys->io_buserr;
			save_context(last_context);
			context_has_been_saved = 1;
			kpanic();
			zip_halt();
		}

		if ((need_resched)||(current->state != SCHED_READY)
			||(current == tasklist[LAST_TASK]))
			current = kschedule(LAST_TASK, tasklist, current);

		if (current->context != last_context) {
			// Swap contexts
			if (!context_has_been_saved)
				save_context(last_context);
			restore_context(current->context);
		}
	} while(1);
}

TASKP	kschedule(int LAST_TASK, TASKP *tasklist, TASKP last) {
	TASKP	current = tasklist[LAST_TASK];
	int nxtid = 0, i;

	// What task were we just running?
	for(i=0; i<=LAST_TASK; i++) {
		if (last == tasklist[i]) {
			// If we found it, then let's run the next one
			nxtid = i+1;
			break;
		}
	}

	// Now let's see if we can find the next ready task to run
	for(; nxtid<LAST_TASK; nxtid++) {
		if (tasklist[nxtid]->state == SCHED_READY) {
			current=tasklist[nxtid];
			break;
		}
	}
	// The last task (the idle task) doesn't count
	if (nxtid >= LAST_TASK) {
		nxtid = 0; // Don't automatically run idle task
		for(; nxtid<LAST_TASK; nxtid++)
			if (tasklist[nxtid]->state == SCHED_READY) {
				break;
			}
		// Now we stop at the idle task, if nothing else is ready
		current = tasklist[nxtid];
	} return current;
}

int	kpost(TASKP *tasklist, unsigned events, int milliseconds) {
	int	i;
	if (events & INT_TIMA)
		milliseconds++;
	if (milliseconds<0) {
		milliseconds -= 0x80000000;
		for(i=0; i<=LAST_TASK; i++) {
			if(tasklist[i]->timeout) {
				tasklist[i]->timeout -= 0x80000000;
				if (tasklist[i]->timeout==0)
					tasklist[i]->timeout++;
				if ((int)tasklist[i]->timeout < milliseconds) {
					tasklist[i]->pending |= SWINT_TIMEOUT;
					tasklist[i]->timeout = 0;
				}
			}
		}
	} else {
		for(i=0; i<=LAST_TASK; i++) {
			if(tasklist[i]->timeout) {
				if (tasklist[i]->timeout < (unsigned)milliseconds) {
					tasklist[i]->pending |= SWINT_TIMEOUT;
					tasklist[i]->timeout = 0;
				}
			}
		}
	} for(i=0; i<=LAST_TASK; i++) {
		tasklist[i]->pending |= events;
		if ((tasklist[i]->state == SCHED_WAITING)
				&&(tasklist[i]->waitsig&tasklist[i]->pending)) {
			tasklist[i]->state = SCHED_READY;
			tasklist[i]->context[1] = tasklist[i]->waitsig & tasklist[i]->pending;
			tasklist[i]->pending &= (~tasklist[i]->context[1]);
			tasklist[i]->waitsig = 0;
		}
	} return milliseconds;
}


