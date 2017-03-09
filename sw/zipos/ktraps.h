////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	ktraps.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	
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
#ifndef	KTRAPS_H
#define	KTRAPS_H

typedef	enum {
	// First two deal with interrupts:
	//	syscall(TRAPID_WAIT, intmask, timeout, ?)
	//		returns when an interrupt in intmask has taken place
	//		may return immediately if such an interrupt has
	//		occurred between the last such syscall and this one
	//		If timeout is != 0, specifies the maximum amount of
	//			time to wait for the event to occurr.
	//		If intmask = 0, then the task will wait for a timeout
	//			alone.
	//		If the task is stalled for another reason, it may wake
	//			early from the trap when the timeout is up.
	//	syscall(TRAPID_CLEAR, intmask, timeout, ?)
	//		Same thing, except this returns immediately after
	//		clearing any potentially pending interrupts
	//		If the timeout < 0, clears any pending timeout wakeup
	//		If the timeout > 0, sets a pending timeout wakeup and
	//			returns.
	//		If the timeout == 0, clears the respective interrupt
	//			slash event, and does nothing more.
	TRAPID_WAIT, TRAPID_CLEAR, TRAPID_POST,
	// Yield: Yields the processor until the next scheduled time slice.
	TRAPID_YIELD,
	TRAPID_READ, TRAPID_WRITE,
	TRAPID_TIME,
	// Return from a kernel system call.  This is necessary if ever an
	// exception triggers a system call.  In such cases, it will be
	// impossible to return the caller back to his context in a pristine
	// manner ... without help.
	// TRAPID_KRETURN,
	// Semaphore ID's.  These allow us to run the rest of the trap
	// stuffs in kernel space
	TRAPID_SEMGET, TRAPID_SEMPUT, TRAPID_SEMNEW,
	// Malloc--since we're using a system level malloc, from a system
	// heap for everything, malloc/free require system calls
	TRAPID_MALLOC, TRAPID_FREE,
	// EXIT -- end a task
	TRAPID_EXIT
} TRAPID;

extern	int	syscall(const int id, const int a, const int b, const int c);

static inline int	read(int fid, void *buf, int ln) {
	return syscall(TRAPID_READ, fid,(int)buf,ln);
} static inline int	write(int fid, const void *buf, int ln) {
	return syscall(TRAPID_WRITE, fid, (int)buf, ln);
}

static inline unsigned	time(void) {
	return syscall(TRAPID_TIME, 0,0,0);
}

static inline	void	yield(void) {
	syscall(TRAPID_YIELD, 0, 0, 0);
} static inline int	wait(unsigned event_mask, int timeout) {
	return syscall(TRAPID_WAIT, (int)event_mask, timeout, 0);
} static inline int	clear(unsigned event, int timeout) {
	return syscall(TRAPID_CLEAR, (int)event, timeout, 0);
} static inline void	post(unsigned event) {
	syscall(TRAPID_POST, (int)event, 0, 0);
}

static inline	void *malloc(unsigned nbytes) {
	return (void *)syscall(TRAPID_MALLOC, (int)nbytes, 0, 0);
} static inline void	free(void *buf) {
	syscall(TRAPID_FREE,(int)buf,0,0);
}

static inline void	exit(int status) {
	syscall(TRAPID_EXIT,status,0,0);
}

#endif
