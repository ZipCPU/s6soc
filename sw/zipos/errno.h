////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	errno.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	Encodes a series of error numbers for the ZipOS.  These are
//		loosely based upon the Linux error codes, but they are by no
//	means complete.  Still ... they are complete enough for what is here.
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
#ifndef	ZIPOS_ERRNO_BASE_H
#define	ZIPOS_ERRNO_BASE_H

#define	EIO	5
#define	EBADF	9
#define	EFAULT	14
#define	EBUSY	16
//
#define	EHEAP	50	// Heap overflow error
#define	EBUS	51	// Bus (memory) error

#endif
