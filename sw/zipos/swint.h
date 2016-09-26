////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	swint.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	Defines the "Software interrupts" that may be created on this
//		system.  A software interrupt is one that is created by
//	software, but may be waited on like any other interrupt.
//
//	The biggest reason for having this is realtime clock (RTC) emulation. 
//	Somewhere, a hardware interrupt needs to prod the software RTC.  Then
//	the software RTC will generate a software interrupt so that anything
//	waiting on the top of the second may now be informed of that fact.
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
#ifndef	SWINT_H
#define	SWINT_H

#define	SWINT_PPS	0x008000
#define	SWINT_TIMEOUT	0x010000
#define	SWINT_PPD	0x020000
#define	SWINT_ALARM	0x040000
#define	SWINT_CLOCK	0x080000
#define	SWINT_DOORBELL	0x100000

#endif
