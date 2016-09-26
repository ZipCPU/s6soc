////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	taskp.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	Implements the new_task function to create a new task, as well
//		as the idle_task's entry function and body.
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
#include "ksched.h"
#include "kfildes.h"
#include "taskp.h"

extern	void *sys_malloc(int sz);
TASKP	new_task(unsigned ln, VENTRYP entry) {
	int	i;
	TASKP	tsk = (TASKP)sys_malloc(sizeof(struct TASK_S)+ln);

	for(i=0; (unsigned)i<sizeof(struct TASK_S)+ln; i++)
		((unsigned int *)tsk)[i] = 0;
	tsk->context[ 0] = (int)((long)(int *)idle_task);
	tsk->context[12] = (int)&tsk->user_data[ln]; // Set the stack pointer
	tsk->context[13] = (int)&tsk->user_data[ln]; // Set the stack pointer
	tsk->context[14] = 0x20; // GIE bit only
	tsk->context[15] = (int)((long)(int *)entry);
	tsk->user_heap   = &tsk->user_data[0];
	tsk->state = SCHED_READY;

	return tsk;
}

// 
// zip_idle is really an assembly language builtin.  It translates into the
// WAIT (i.e. for interrupt) instruction.  Even this, though, is a derived
// instruction.  More specifically, the WAIT instruction OR's a constant
// to the CC register.
//
extern void zip_idle(void);
void	idle_task(void) {
	do {
		zip_idle();
	} while(1);
}

