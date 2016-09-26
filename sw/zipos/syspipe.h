////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	syspipe.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	This "device" handles the primary device level interaction of
//		almost all devices on the ZipOS: the pipe.  A pipe, as defined
//	here, is an O/S supported FIFO.  Information written to the FIFO will
//	be read from the FIFO in the order it was received.  Attempts to read
//	from an empty FIFO, or equivalently to write to a full FIFO, will block
//	the reading (writing) process until memory is available.
//
//	In general, each PIPE has three interface functions for reading and
//	another three for writing, together with one for creating an empty pipe.
//	The read functions are:
//
//		kpop_syspipe
//			Attempts to read one value from the pipe in an interrupt
//			context.  It is not allowed to fail.  An empty pipe
//			simply becomes a return value of '1'.
//
//		kread_syspipe
//			This is where the user trap read() ends up.  When a user
//			tries to read from a pipe, kread_syspipe checks that the
//			users request is valid, then transfers control to a
//			user task level (interrupts enabled) read function.
//
//		uread_syspipe
//			The user read task.  This task is invoked from the user
//			trap, calling kread_syspipe at the kernel mode, which
//			then transitions the read to user mode using
//			uread_syspipe.
//
//	Writing has it's analogous functions:
//		kpush_syspipe
//			Attempts to write a value from an interrupt context
//			to the pipe.  Primarily used when reading from a 
//			UART driven device.
//		kwrite_syspipe
//		uwrite_syspipie
//
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
#ifndef	SYSPIPE_H
#define	SYSPIPE_H

extern	void *sys_malloc(int sz);

typedef struct {
	unsigned int	m_mask;
	int		m_head, m_tail;
volatile TASKP		m_rdtask, m_wrtask;
	unsigned int	m_nread, m_nwritten;
	int	dummy;
	int		m_error;

	int		m_buf[1];
} SYSPIPE;

SYSPIPE *new_syspipe(const unsigned int len);
extern	void	kread_syspipe( TASKP tsk, int dev, int *dst, int len);
extern	void	kwrite_syspipe(TASKP tsk, int dev, int *src, int len);
void	kpush_syspipe(SYSPIPE *p, int val);
int	kpop_syspipe(SYSPIPE *p, int *val);
extern	int	num_avail_syspipe(SYSPIPE *p);
extern	int	len_syspipe(SYSPIPE *p);

#define	INTERRUPT_READ_TASK	((TASKP)((unsigned)-1))
// #define	INTERRUPT_WRITE_TASK	((TASKP)((unsigned)-1))

#endif
