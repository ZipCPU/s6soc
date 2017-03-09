////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	asmstartup.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	A small assembly routine, included from C via the "asm" 
//		statement, to place startup code into the very beginning of
//	the program space.  This startup code *must* start at the RESET_ADDRESS
//	of the ZipCPU.  It does two things: 1) loads a valid stack pointer, and
//	2) jumps to the entry point in the program which (as a result of this
//	startup code) may be anywhere in the address space.
//
//	Sadly, the main program often follows this code, leading to a long jump
//	to the main program entry.  This isn't that efficient, but ... on a
//	non-pipelined machine it shouldn't cost more than one extra flash
//	access.
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
#ifndef	ASMSTARTUP_H
#define	ASMSTARTUP_H

asm("\t.section\t.start\n"
"\t.global\t_start\n"
"\t.type\t_start,@function\n"
"_start:\n"
"\tNDUMP\n"
"\tMOV\tkernel_exit(PC),uPC\n"
"\tLDI\t255,R0\n"
"\tSW\tR0,0x414\n"
"\tLDI\t_top_of_stack,SP\n"
"\tJSR\tentry\n"
"NEXIT\tR0\n"
"kernel_exit:\n"
"\tHALT\n"
"\tBRA\tkernel_exit\n"
"\t.section\t.text");

#endif
