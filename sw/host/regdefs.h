////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	regdefs.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	This file defines C constants which can be used when
//		communicating with the FPGA device from the PC host.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015-2017, Gisselquist Technology, LLC
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
#ifndef	REGDEFS_H
#define	REGDEFS_H

#define	R_ICONTROL	0x00000400
#define	R_BUSERR	0x00000404
#define	R_ITIMERA	0x00000408
#define	R_ITIMERB	0x0000040c
#define	R_PWM		0x00000410
#define	R_SPIO		0x00000414
#define	R_GPIO		0x00000418
#define	R_UART		0x0000041c
#define	R_VERSION	0x00000420

// WB Scope registers
#define	R_SCOPE		0x00000800
#define	R_SCOPED	0x00000804
//
// And because the flash driver needs these constants defined ...
#define	R_QSPI_EREG	0x00000c00
#define	R_QSPI_CREG	0x00000c04
#define	R_QSPI_SREG	0x00000c08
#define	R_QSPI_IDREG	0x00000c0c
//

// RAM memory space
#define	LGMEMSZ		14
#define	RAMBASE		(1<<LGMEMSZ)
#define	MEMWORDS	(1<<(LGMEMSZ-1))
#define	RAMLEN		(1<<LGMEMSZ)

// Flash memory space
#define	LGFLASHSZ	24		// Log_2 of the number of bytes in flash
#define	SPIFLASH	(1<<LGFLASHSZ)
#define	FLASHWORDS	(1<<(LGFLASHSZ-2))
#define	FLASHLEN	(1<<LGFLASHSZ)
#define	CONFIG_ADDRESS	  SPIFLASH // Main Xilinx configuration (ZipCPU)
#define	ALTCONFIG_ADDRESS (SPIFLASH+0x100000) // Alt Xilinx config (Dbg)
#define	RESET_ADDRESS	  (SPIFLASH+0x200000) // ZipCPU Reset address

// Interrupt control constants
#define	GIE		0x80000000	// Enable all interrupts
#define	SCOPEN		0x80040004	// Enable WBSCOPE interrupts
#define	ISPIF_EN	0x88000800	// Enable SPI Flash interrupts
#define	ISPIF_DIS	0x08000000	// Disable SPI Flash interrupts
#define	ISPIF_CLR	0x08000800	// Clear pending SPI Flash interrupt

// Flash control constants
#define	ERASEFLAG	0x80000000
#define	DISABLEWP	0x10000000
#define	ENABLEWP	0x00000000

// Sectors are defined as 64 kB (16 kW)
#define	SZPAGEB		256
#define	PGLENB		256
#define	SZPAGEW		64
#define	PGLENW		64
#define	NPAGES		256	// 64 kB sectors / 256 bytes is ...
#define	SECTORSZB	(NPAGES * SZPAGEB)	// In bytes, not words!!
#define	SECTORSZW	(NPAGES * SZPAGEW)	// In words
#define	NSECTORS	(FLASHLEN/SECTORSZB)	// 256 sectors
#define	SECTOROF(A)	((A) & (-1<<16))
#define	PAGEOF(A)	((A) & (-1<<8))

// Scop definition/sequences
#define	SCOPE_NO_RESET	0x80000000
#define	SCOPE_TRIGGER	(0x08000000|SCOPE_NO_RESET)
#define	SCOPE_MANUAL	SCOPE_TRIGGER
#define	SCOPE_DISABLE	(0x04000000)

typedef	struct {
	unsigned	m_addr;
	const char	*m_name;
} REGNAME;

extern	const	REGNAME	*bregs;
extern	const	int	NREGS;
// #define	NREGS	(sizeof(bregs)/sizeof(bregs[0]))

extern	unsigned	addrdecode(const char *v);
extern	const	char *addrname(const unsigned v);

// #include "ttybus.h"
// #include "portbus.h"
// #include "deppbus.h"

// typedef	DEPPBUS	FPGA;
#include "ttybus.h"
typedef	TTYBUS	FPGA;

#endif
