/*******************************************************************************
*
* Filename:	cmod.ld
*
* Project:	Cmod S6 ZipCPU demonstration
*
* Purpose:	This script provides a description of the Cmod S6 Zip CPU
*		build for the purposes of where to place memory when linking.
*
* Creator:	Dan Gisselquist, Ph.D.
*		Gisselquist Technology, LLC
*
********************************************************************************
*
* Copyright (C) 2016, Gisselquist Technology, LLC
*
* This program is free software (firmware): you can redistribute it and/or
* modify it under the terms of  the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or (at
* your option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* License:	GPL, v3, as defined and found on www.gnu.org,
*		http://www.gnu.org/licenses/gpl.html
*
*
*******************************************************************************/

ENTRY(_start)

MEMORY
{
	blkram (wx) : ORIGIN = 0x002000, LENGTH = 0x001000
	flash  (rx) : ORIGIN = 0x400000, LENGTH = 0x400000
}

_top_of_stack = ORIGIN(blkram) + LENGTH(blkram) - 1;

SECTIONS
{
  . = 0x0480000;
  .rocode 0x0480000 : { *(.start) *(.text)
	*(.rodata)
	*(.strings) } > flash
  .data : { *(.fixdata) *(.data) *(COMMON) *(.bss) } > blkram
  _top_of_heap = .;
}
