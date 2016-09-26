////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	bootloader.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To copy into RAM, upon boot, sections from the FLASH that
//		need to be placed into RAM.  This also handles setting initial
//		values.
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
#include "board.h"

// These values will be filled in by the linker.  They are unknown at compile
// time.
extern	int	load_image_start, load_image_end, bss_image_end;

void	bootloader(void) {
	int	len = ((int)&load_image_end) - RAMADDR;
	int	*flash = &load_image_start;
	int	*mem = (int *)RAMADDR;

	for(int i=0; i<len; i++)
		mem[i] = flash[i];
	// While I'd love to continue and clear to the end of memory, doing
	// so will corrupt my stack and perhaps even my return address.  Hence
	// we only do this much.
	for(int i=len; i< ((int)&bss_image_end)-RAMADDR; i++)
		mem[i] = 0;
}

