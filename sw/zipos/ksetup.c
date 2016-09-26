////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	ksetup.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	These are the routines from kernel.c that didn't need to be
//		in RAM memory.  They are specifically the pre-run setup
//	routines that are executed from FLASH memory.
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

extern int	kntasks(void);
extern void	kinit(TASKP *tasklist);
extern	void	restore_context(int *), save_context(int *);
SYSPIPE	*rxpipe, *txpipe, *keypipe, *lcdpipe, *pwmpipe, *cmdpipe;
KDEVICE	*pipedev, *txdev, *pwmdev;
void	*heap; //  = _top_of_heap; // Need to wait on startup to set this

#define	CONTEXT_LENGTH	(80000-1)	// 1ms

int	LAST_TASK;

__attribute__((cold))
TASKP	*ksetup(void) {
	TASKP	*tasklist;
	IOSPACE	*sys = (IOSPACE *)IOADDR;

	sys->io_spio = 0x0f0;
	sys->io_timb = 0;	// Turn off the watchdog timer
	LAST_TASK = kntasks();
	heap = _top_of_heap;

	pipedev = sys_malloc(sizeof(KDEVICE));
	pipedev->write = (RWFDFUN)kwrite_syspipe;
	pipedev->read  = (RWFDFUN)kread_syspipe;
	pipedev->close = NULL;

	txdev = pwmdev = pipedev;

	rxpipe  = new_syspipe(16);	//rxpipe->m_wrtask=INTERRUPT_WRITE_TASK;
	txpipe  = new_syspipe(8);	txpipe->m_rdtask = INTERRUPT_READ_TASK;
	keypipe = new_syspipe(16);
	lcdpipe = new_syspipe(16);
	pwmpipe = new_syspipe(128);	pwmpipe->m_rdtask= INTERRUPT_READ_TASK;
	cmdpipe = new_syspipe(16);

	tasklist = sys_malloc(sizeof(TASKP)*(1+LAST_TASK));
	kinit(tasklist);
	tasklist[LAST_TASK] = new_task(2, idle_task);

	// Turn all interrupts off, acknowledge all at the same time
	sys->io_pic = 0x7fff7fff;

	sys->io_tima = CONTEXT_LENGTH | TM_REPEAT;

	{
		// Reset our wishbone scope for debug later
		SCOPE	*scope = (SCOPE *)SCOPEADDR;
		scope->s_control = 2;
	}
	sys->io_spio = 0x0f1;

	return tasklist;
}

void	kwait_on_buttonpress(void) {
	IOSPACE	*sys = (IOSPACE *)IOADDR;

	// Wait on a button press before starting
	while((sys->io_spio & 0x0f0)==0)
		;
	sys->io_spio = 0x0f3;
	for(int i=0; i<40000; i++)
		sys->io_spio = ((i>>14)&2)|0x020;
	sys->io_spio = 0x0f7;
}

// __attribute__((noreturn))
void	kuserexit(int a) {
	syscall(TRAPID_EXIT, a, 0, 0);
}

__attribute__((malloc))
void	*sys_malloc(int sz) {
	if (0) {
		SCOPE	*s = (SCOPE *)SCOPEADDR;
		s->s_control = TRIGGER_SCOPE_NOW | (s->s_control & 0x0ffff);
	}

	void	*res = heap;
	heap += sz;
	if ((int)heap > ((int)&res)-32) {
		IOSPACE	*sys = (IOSPACE *)IOADDR;
		sys->io_spio = 0xf3;
		asm("break 0");
	}
	return res;
}

KFILDES *kopen(int id, KDEVICE *dev) {
	KFILDES	*fd = (KFILDES *)sys_malloc(sizeof(KFILDES));
	fd->id = (int)id;
	fd->dev= dev;
	return fd;
}
