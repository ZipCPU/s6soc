////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	display.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	A device driver for the SPI controlled display.  The primary
//		purpose of this "device" is to output SPI instructions, though,
//	not to handle screen position or to optimize data set to the display.
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
#ifndef	DISPLAY_H
#define	DISPLAY_H

#define	PACK(A,B,C,D)	(((A)<<24)|((B)<<16)|((C)<<8)|(D))
extern void	dispchar(int ch);

#ifdef	ZIPOS
extern void	displaytask(void);
#endif	// ZIPOS
#endif	// DISPLAY_H
