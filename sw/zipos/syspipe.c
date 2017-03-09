////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	syspipe.c
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
#include "string.h"

#ifndef	NULL
#define	NULL	(void *)0
#endif

void	kpush_syspipe(SYSPIPE *pipe, char val) {
	int	tst = (pipe->m_head+1)&pipe->m_mask;
	if (tst != pipe->m_tail) {
		pipe->m_buf[pipe->m_head] = val;
		pipe->m_head = tst;		// Increment the head pointer
		if ((pipe->m_rdtask)&&(pipe->m_rdtask != INTERRUPT_READ_TASK))
			pipe->m_rdtask->state = SCHED_READY;
	} else pipe->m_error = 1;
}

extern	void	pipe_panic(SYSPIPE *p);
/*
 * kpop_syspipe
 *
 * Called from an interrupt context to pop one byte off of the syspipe.
 */
int	kpop_syspipe(SYSPIPE *pipe, char *vl) {
	if (pipe->m_head != pipe->m_tail) {
		// The head will *only* equal the tail if the pipe is empty.
		// We come in here, therefore, if the pipe is non-empty
		*vl = pipe->m_buf[pipe->m_tail];
		pipe->m_tail = (pipe->m_tail+1)&pipe->m_mask;
		if (pipe->m_wrtask)
			pipe->m_wrtask->state = SCHED_READY;
		return 0;
	}
	return 1; // Error condition
}

/*
 * kpop_short_syspipe
 *
 * This is identical to kpop_syspipe, save that we are pulling a short value
 * off of the syspipe (i.e. a pair of chars), not just a single char.
 */
int	kpop_short_syspipe(SYSPIPE *pipe, unsigned short *vl) {
	if (pipe->m_head == pipe->m_tail)
		return 1; // Error condition

	unsigned short *sptr = (unsigned short *)&pipe->m_buf[pipe->m_tail];
	// sv = pipe->m_buf[pipe->m_tail];
	*vl = *sptr;;
	pipe->m_tail = (pipe->m_tail+2)&pipe->m_mask;
	if (pipe->m_wrtask)
		pipe->m_wrtask->state = SCHED_READY;
	return 0;
}

/*
int	len_syspipe(SYSPIPE *p) {
	return (p->m_head-p->m_tail) & p->m_mask;
} */

/* Returns how many empty spaces are in the pipe
 */
int	num_avail_syspipe(SYSPIPE *p) {
	// if (head+1 == tail)	PIPE is full
	//	(mask+tail-tail+1)=mask+1 &mask = 0
	// if (head == tail) PIPE is empty
	//	(mask+tail-tail)=mask & mask = mask
	// if (head == tail+2) PIPE has two within it
	//	(mask+tail-tail-2)=mask-2 & mask = mask-2
	//
	return (p->m_tail-p->m_head-1) & p->m_mask;
}

// This will be called from a user context.
// Another task may write to the pipe during this call.  If the pipe becomes
// full, that task will block.
//
static int uread_syspipe(TASKP tsk __attribute__((__unused__)),
		SYSPIPE *p, char *dst, int len) {
	int nleft= len, h;
	if (len == 0) {
		// We'll only get here if we were released from within a 
		// writing task.
		return p->m_nread;
	} else do {
		// We have a valid read request, for a new process.  Continue
		// 'reading' until we have fulfilled the request.
		//
		// We can read from head, just not write to it
		// As for the tail pointer -- we own it, no one else can touch
		// it.
		h = ((volatile SYSPIPE *)p)->m_head;
		if (h < p->m_tail) {
			// The buffer wraps around the end.  Thus, we first
			// read anything between the tail pointer and the end
			int ln1 = p->m_mask+1 - p->m_tail; // Navail to be read
			ln1 = (ln1 > nleft) ? nleft : ln1;
			if (ln1 > 0) {
				memcpy(dst, &p->m_buf[p->m_tail], ln1);
				dst += ln1;

				p->m_nread += ln1;
				nleft -= ln1;

				int nt = p->m_tail+ln1;
				if ((unsigned)nt > p->m_mask)
					nt = 0;
				p->m_tail = nt;
			}

			// nleft is either zero, or tail
			if (nleft & -2)
				exit(nleft);
			else if (p->m_nread & -2)
				exit(p->m_nread);
		}

		// Then repeat with the second half of the buffer, from the
		// beginning to the head--unless we've exhausted our buffer.
		if (nleft > 0) {
			// Still need to do more, wrap around our buffer and
			// restart
			int ln1 = h - p->m_tail;
			ln1 = (ln1 < nleft) ? ln1 : nleft;

			memcpy(dst, &p->m_buf[p->m_tail], ln1);
			dst += ln1;

			p->m_nread += ln1;
			nleft -= ln1;
			int nt = p->m_tail+ln1; // nt = new tail value
			if ((unsigned)nt > p->m_mask)
				nt = 0;
			p->m_tail = nt;

			if (nleft & -2)
				exit(nleft);
			else if (p->m_nread & -2)
				exit(p->m_nread);
		}

		if (nleft == 0)
			break;

		// We didn't finish our read, check for a blocked writing
		// process to copy directly from.  Note that we don't need
		// to check the status of the write task--if it is set and
		// we are active, then it is blocked and waiting for us to
		// complete.  Note also that this is treated as a volatile
		// pointer.  It can change from one time through our loop
		// to the next.
		if (((volatile SYSPIPE *)p)->m_wrtask) {
			int	ln;
			char	*src;

			// If the head changed before the write task blocked,
			// then go around again and copy some more before
			// getting started
			//
			// This should never happen, however.  If a write task
			// gets assigned while a read task exists, it doesn't
			// write its values into the buffer, it just waits.
			// therefore we don't need to check for this.
			//
			// if (p->m_head != h)
			//	continue;

			ln = nleft;
			if (p->m_wrtask->context[4] < nleft)
				ln = p->m_wrtask->context[4];
			src = (char *)p->m_wrtask->context[3];
			memcpy(dst, src, ln);
			dst += ln;
			src += ln;

			p->m_nwritten += ln;
			p->m_nread    += ln;

			nleft -= ln;
			p->m_wrtask->context[4] -= ln;
			p->m_wrtask->context[3]  = (int)src;

			// We have exhausted the write task.  Release it
			if (p->m_wrtask->context[4] == 0) { // wr_len == 0
				// Release the write task, it has exhausted
				// its buffer
				TASKP	w = p->m_wrtask;
				// Now we allow other tasks to write into our
				// pipe
				p->m_wrtask = 0;
				// And here we actually release the writing
				// task
				w->state = SCHED_READY;
			}
		}

		// Realistically, we need to block here 'till more data is
		// available.  Need to determine how to do that.  Until then,
		// we'll just tell the scheduler to yield.  This will in
		// effect create a busy wait--not what we want, but it'll work.
		if (nleft > 0) {
			DISABLE_INTS();
			h = ((volatile SYSPIPE *)p)->m_head;
			if (h == p->m_tail)
				wait(0,-1);
			else
				ENABLE_INTS();
		}
	} while(nleft > 0);

	len = p->m_nread;
	p->m_nread = 0;
	// Release our ownership of the read end of the pipe
	DISABLE_INTS();
	p->m_rdtask = NULL;
	if (((volatile SYSPIPE *)p)->m_wrtask)
		p->m_wrtask->state = SCHED_READY;
	ENABLE_INTS();

	// We have accomplished our read
	//
	return len;
}

static int	uwrite_syspipe(TASKP tsk __attribute__((__unused__)),
		SYSPIPE *p, char *src, int len) {
	int nleft = len;

	// The kernel guarantees, before we come into here, that we have a 
	// valid write request.  
	do {
		// We try to fill this request without going through the pipes
		// memory at all.  Hence, if there is a read task that is
		// waiting/suspended, waiting on a write (this must've happened
		// since we started)--write directly into the read buffer first.

		// If there is a read task blocked, the pipe must be empty
		TASKP rdtask = ((volatile SYSPIPE *)p)->m_rdtask;
		if (rdtask == INTERRUPT_READ_TASK) {
			// We need to copy everything to the buffer
		} else if (rdtask) {
			int ln = nleft;
			if (ln > p->m_rdtask->context[4])
				ln = p->m_rdtask->context[4];
			memcpy((char *)p->m_rdtask->context[3], src, ln);
			src += ln;
			p->m_nread += ln;
			p->m_rdtask->context[3]+= ln;
			p->m_rdtask->context[4]-= ln;
			nleft -= ln;
			p->m_nwritten += ln;

			// Realistically, we always need to wake up the reader
			// at this point.  Either 1) we exhausted the readers
			// buffer, or 2) we exhausted our own and the reader
			// needs to take over.  Here, we only handle the first
			// case, leaving the rest for later.
			if (p->m_rdtask->context[4] == 0) {
				TASKP	r = p->m_rdtask;
				// Detach the reader task
				p->m_rdtask = 0;
				// Wake up the reader
				r->state = SCHED_READY;
			}

			// While it might appear that we might close our loop
			// here, that's not quite the case.  It may be that the
			// pipe is read from an interrupt context.  In that
			// case, there will never be any reader tasks, but we
			// will still need to loop.

			// Now that we've filled any existing reader task, we
			// check whether or not we fit into the buffer.  The
			// rule is: don't write into the buffer unless
			// everything will fit.  Why?  Well, if you have to
			// block anyway, why not see if you can't avoid a
			// double copy?
			if (nleft == 0)
				break;
		}

		// Copy whatever we have into the pipe's buffer
		int	navail = num_avail_syspipe(p);
		if ((nleft <= navail)
			||((rdtask == INTERRUPT_READ_TASK)&&(navail>0))) {
			// Either there is no immediate reader task, or
			// the reader has been exhausted, but we've go
			// more to write.
			//
			// Note that we no longer need to check what
			// will fit into the pipe.  We know the entire
			// rest of our buffer will fit.

			{ // Write into the first half of the pipe
			// Be careful not to change head until all is written
			// so that it remains consistent under interrupt
			// conditions.
				int ln = p->m_mask+1-p->m_head;
				if (ln > nleft) ln = nleft;
				if (ln > navail) ln = navail;

				memcpy((void *)&p->m_buf[p->m_head], src, ln);
				src += ln;

				p->m_head = (p->m_head+ln)&p->m_mask;
				nleft		-= ln;
				p->m_nwritten	+= ln;
				navail		-= ln;
			}
		}

		if ((nleft > 0)&&(navail == 0)) {
			if (rdtask == INTERRUPT_READ_TASK) {
				DISABLE_INTS();
				if (0==num_avail_syspipe(p))
					wait(0,-1);
				else ENABLE_INTS();
			} else {
				DISABLE_INTS();
				if (!((volatile SYSPIPE *)p)->m_rdtask)
					wait(0,-1); // Should really be a wait
					// on JIFFIES and if JIFFIES expired
					// (i.e. write timeout) then break;
				else ENABLE_INTS();
			}
		}
	} while(nleft > 0);

	int	nw= p->m_nwritten;
	p->m_wrtask = 0;
	return nw;
}

// This will be called from a kernel (interrupt) context
void	kread_syspipe(TASKP tsk, int dev, char *dst, int len) {
	SYSPIPE	*p = (SYSPIPE *)dev;
	if (p->m_rdtask != NULL) {
		// If the pipe already has a read task, then we fail
		tsk->context[1] = -EBUSY;
		zip_halt();
	} else if (p->m_error) {
		// If there's been an overrun, let the reader know on the
		// next read--i.e. this one.  Also, clear the error condition
		// so that the following read will succeed.
		tsk->context[1] = -EIO;
		p->m_tail = p->m_head;
		p->m_error = 0;
	} else if (len <= 0) {
		tsk->context[1] = -EFAULT;
		zip_halt();
	} else if (!valid_ram_region(dst, len)) {
		// Bad parameters
		tsk->context[1] = -EFAULT;
		zip_halt();
	} else {
		// Take  ownership of the read end of the pipe
		p->m_rdtask = tsk;
		p->m_nread  = 0;
		tsk->context[1] = (int)tsk;
		tsk->context[2] = (int)p;
		// These are  already set, else we'd set them again
		// tsk->context[3] = (int)dst;
		// tsk->context[4] = len;
		tsk->context[15] = (int)uread_syspipe;

		// If there is already a write task, make sure it is awake
		if (p->m_wrtask) {
			tsk->state = SCHED_WAITING;
			p->m_wrtask->state = SCHED_READY;
		} else if (p->m_head == p->m_tail)
			// If the pipe is empty, block the read task
			tsk->state = SCHED_WAITING;

		// On return, this will bring us back to user space, inside our
		// user space version of the read system call
	}
}

void	kwrite_syspipe(TASKP tsk, int dev, char *src, int len) {
	SYSPIPE	*p = (SYSPIPE *)dev;
	if (p->m_wrtask != NULL) {
		// If the pipe already has a write task, then we fail
		tsk->context[1] = -EBUSY;
	} else if (len <= 0) {
		tsk->context[1] = -EFAULT;
	} else if (!valid_mem_region(src, len)) {
		// Bad parameters
		tsk->context[1] = -EFAULT;
		zip_halt();
	} else {
		// Take  ownership of the write end of the pipe
		p->m_wrtask    = tsk;
		p->m_nwritten = 0;
		tsk->context[1] = (int)tsk;
		tsk->context[2] = (int)p;
		// These are  already set, else we'd set them again
		// tsk->context[3] = (int)src;
		// tsk->context[4] = len;
		tsk->context[15] = (int)uwrite_syspipe;

		// If a reader task currently exists, then block until that
		// task either finishes or releases us
		if ((p->m_rdtask)&&(p->m_rdtask != INTERRUPT_READ_TASK)) {
			tsk->state = SCHED_WAITING;
			p->m_rdtask->state = SCHED_READY;
		} else if (((p->m_head+1)&p->m_mask) == (unsigned)p->m_tail)
			// If the pipe is empty, block until there's data
			tsk->state = SCHED_WAITING;

		// On return, this will bring us back to user space, in our
		// user space write call
	}
}
