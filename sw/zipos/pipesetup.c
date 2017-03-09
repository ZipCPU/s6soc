////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	pipesetup.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	The routines in this file were split from syspipe.c for the
//		purposes of limiting the amount of RAM memory a process would
//	use.  Specifically, these routines are not time critical and hence can
//	run from FLASH, whereas the other set must run from block RAM.
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
#include "errno.h"
#include "board.h"
#include "taskp.h"
#include "syspipe.h"
#include "zipsys.h"
#include "ktraps.h"
#include "txfns.h"

#ifndef	NULL
#define	NULL	(void *)0
#endif

// Since this is only called with new_syspipe, we're good here
static	void	clear_syspipe(SYSPIPE *p) {
	p->m_head  = 0;
	p->m_tail  = 0;
	p->m_error = 0;

	for(int i=0; i<=(int)p->m_mask; i++)
		p->m_buf[i] = 0;

	if ((p->m_rdtask)&&(p->m_rdtask != INTERRUPT_READ_TASK)) {
		p->m_rdtask->context[1] = p->m_nread;
		if (p->m_nread == 0)
			p->m_rdtask->errno = -EFAULT;
		p->m_rdtask->state = SCHED_READY;
	} else if (p->m_wrtask) {
		p->m_wrtask->context[1] = p->m_nwritten;
		if (p->m_nwritten == 0)
			p->m_wrtask->errno = -EFAULT;
		p->m_wrtask->state = SCHED_READY;
	}

	if (p->m_rdtask != INTERRUPT_READ_TASK)
		p->m_rdtask   = 0;
	p->m_wrtask   = 0;
	p->m_nread    = 0;
	p->m_nwritten = 0;
}

void	pipe_panic(SYSPIPE *pipe) {
	extern void	kpanic(void);

	_sys->io_spio = 0x0fa;
	
	txstr("SYSPIPE PANIC!\r\n");
	txstr("ADDR: "); txhex((int)pipe);
	txstr("MASK: "); txhex(pipe->m_mask);
	txstr("HEAD: "); txhex(pipe->m_head);
	txstr("TAIL: "); txhex(pipe->m_tail);
	kpanic();
}

SYSPIPE *new_syspipe(const unsigned int len) {
	unsigned	msk;

	for(msk=16; msk<len; msk<<=1)
		;
	SYSPIPE *pipe = sys_malloc(sizeof(SYSPIPE)-1+msk);
	pipe->m_mask = msk-1;
	pipe->m_rdtask = pipe->m_wrtask = 0;
	clear_syspipe(pipe);
	return pipe;
}

