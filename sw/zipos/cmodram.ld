/*******************************************************************************
*
* Filename:	cmodram.ld
*
* Project:	Cmod S6 ZipCPU demonstration
*
* Purpose:	This script provides a description of the Cmod S6 Zip CPU
*		build for the purposes of where to place memory when linking.
*
*	This script is different from the cmod.ld script in that this script
*	places specific pieces of code into RAM rather than FLASH.  This is to
*	speed up those particular pieces of code.  This script also depends
*	upon a bootloader to load the RAM sections into RAM with their initial
*	values.
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
	blkram (wx) : ORIGIN = 0x0004000, LENGTH = 0x0004000
	flash  (rx) : ORIGIN = 0x1000000, LENGTH = 0x1000000
}

_flash	= ORIGIN(flash);
_blkram	= ORIGIN(blkram);
_sdram	= 0;
_top_of_stack = ORIGIN(blkram) + LENGTH(blkram) - 4;
_sdram_image_start = 0;
_sdram_image_end   = 0;

SECTIONS
{
	.rocode 0x1200000 : ALIGN(4) {
		_boot_address = .;
		*(.start) *(.boot)
		obj-zip/bootloader.o(.text)
		obj-zip/ksetup.o(.text)
		obj-zip/pipesetup.o(.text)
		obj-zip/taskp.o(.text)
		obj-zip/keypad.o(.text*)
		obj-zip/display.o(.text*)
		obj-zip/rtcsim.o(.text*)
		obj-zip/doorbell.o(.text*)
		*(.rodata*)
		*(.strings*)
		__rocode_alignment = (. + 3) & ~ 3;
		. = __rocode_alignment;
		} > flash
	_kernel_image_start = . ;
 	.ramcode : ALIGN_WITH_INPUT {
		obj-zip/kernel.o(.text*)
		obj-zip/syspipe.o(.text*)
		} > blkram AT> flash
	.data : ALIGN_WITH_INPUT {
		*(.kernel*)
		*(.fixdata*)
		*(.data*)
		*(COMMON*)
		_kernel_image_end = . ;
		}> blkram AT> flash
	_blkram_image_end = . ;
	.bss : ALIGN_WITH_INPUT {
		*(.bss*)
		_bss_image_end = . ;
		} > blkram
	_top_of_heap = .;
}
