////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	kfildes.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	Defines a "File Descriptor" from the standpoint of the kernel.
//		Inside the kernel, each process has an array of these file
//	descriptors.  Each file descriptor contains a pointer to the file
//	itself (typically a SYSPIPE), as well as to a device structure of
//	function pointers that can be used to operator on the file pointer.
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
#ifndef	KFILDES_H
#define	KFILDES_H

struct	TASK_S;

#define	FILENO_STDIN	0
#define	FILENO_STDOUT	1
#define	FILENO_STDERR	2
#define	FILENO_AUX	3
#define	MAX_FILDES	4

typedef	void	(*RWFDFUN)(struct TASK_S *,int, void *, int);

typedef	struct	{
	void	(*write)(struct TASK_S *tsk, int id, void *buf, int len);
	void	(*read)( struct TASK_S *tsk, int id, void *buf, int len);
	void	(*close)(struct TASK_S *tsk, int id);
} KDEVICE;

typedef struct {
	int	id;
	KDEVICE	*dev;
} KFILDES;

#ifndef	NULL
#define	NULL	(void *)0
#endif

KFILDES	*kopen(int id, KDEVICE *dev);

#endif
