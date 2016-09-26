////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	taskp.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	Defines the components of a task structure, containing all the
//		resources necessary to comprehend a user task: memory, state,
//	interrupt information, etc.
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
#ifndef	TASKP_H
#define	TASKP_H

#include "zipsys.h"
#include "ksched.h"
#include "kfildes.h"

#define	MAX_KFILDES	4

typedef	struct TASK_S {	// Cost: 22+user_data
	KSCHED_STATE	state;
	int	context[16];
	KFILDES *fd[MAX_KFILDES];
	int	*user_heap, errno;
	// Interrupt processing.  Waitsig is a list of interupts the task
	// wishes to be woken for.  Pending is the list it hasn't yet seen.
	int	waitsig, pending;
	// Similarly, timeout is when the task wishes to be woken up (if set).
	unsigned	timeout;
	int	user_data[1];
	//
} TASK, *TASKP;

typedef	void (*VENTRYP)(void);
typedef	void (*FENTRYP)(int a, int b, int c, int d);

//
// Create a new task with ln words of memory and entry point entry.
//
TASKP	new_task(unsigned ln, VENTRYP entry);

//
// The entry point for the "special" idle task, whose only purpose is to call
// the idle function as often as possible.  (The CPU does nothing in idle mode)
// Calling this task will prevent the CPU from doing other more useful things,
// so it is the lowest priority task.
//
extern void	idle_task(void);

#endif
