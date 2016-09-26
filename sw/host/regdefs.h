////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	regdefs.h
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
#ifndef	REGDEFS_H
#define	REGDEFS_H

#define	R_VERSION	0x00000108
#define	R_ICONTROL	0x00000100
#define	R_BUSERR	0x00000101
#define	R_ITIMERA	0x00000102
#define	R_ITIMERB	0x00000103
#define	R_PWM		0x00000104
#define	R_SPIO		0x00000105
#define	R_GPIO		0x00000106
#define	R_UART		0x00000107

// WB Scope registers
#define	R_SCOPE		0x00000200
#define	R_SCOPED	0x00000201
//
// And because the flash driver needs these constants defined ...
#define	R_QSPI_EREG	0x0000030c
#define	R_QSPI_CREG	0x0000030d
#define	R_QSPI_SREG	0x0000030e
#define	R_QSPI_IDREG	0x0000030f
//
// FPGA CONFIG/ICAP REGISTERS
#define	R_CFG_CRC	0x00000400
#define	R_CFG_FAR_MAJ	0x00000401
#define	R_CFG_FAR_MIN	0x00000402
#define	R_CFG_FDRI	0x00000403
#define	R_CFG_FDRO	0x00000404
#define	R_CFG_CMD	0x00000405
#define	R_CFG_CTL	0x00000406
#define	R_CFG_MASK	0x00000407
#define	R_CFG_STAT	0x00000408
#define	R_CFG_LOUT	0x00000409
#define	R_CFG_COR1	0x0000040a
#define	R_CFG_COR2	0x0000040b
#define	R_CFG_PWRDN	0x0000040c
#define	R_CFG_FLR	0x0000040d
#define	R_CFG_IDCODE	0x0000040e
#define	R_CFG_CWDT	0x0000040f
#define	R_CFG_HCOPT	0x00000410
#define	R_CFG_CSBO	0x00000412
#define	R_CFG_GEN1	0x00000413
#define	R_CFG_GEN2	0x00000414
#define	R_CFG_GEN3	0x00000415
#define	R_CFG_GEN4	0x00000416
#define	R_CFG_GEN5	0x00000417
#define	R_CFG_MODE	0x00000418
#define	R_CFG_GWE	0x00000419
#define	R_CFG_GTS	0x0000041a
#define	R_CFG_MFWR	0x0000041b
#define	R_CFG_CCLK	0x0000041c
#define	R_CFG_SEU	0x0000041d
#define	R_CFG_EXP	0x0000041e
#define	R_CFG_RDBK	0x0000041f
#define	R_CFG_BOOTSTS	0x00000420
#define	R_CFG_EYE	0x00000421
#define	R_CFG_CBC	0x00000422
// RTC clock control
#define	R_CLOCK		0x00000800
#define	R_TIMER		0x00000801
#define	R_STOPWATCH	0x00000802
#define	R_CKALARM	0x00000803

// RAM memory space
#define	RAMBASE		0x00002000
#define	MEMWORDS	(1<<12)
#define	RAMLEN		MEMWORDS

// Flash memory space
#define	SPIFLASH	0x00400000
#define	FLASHWORDS	(1<<22)
#define	CONFIG_ADDRESS	0x00400000 // Main Xilinx configuration (ZipCPU)
#define	ALTCONFIG_ADDRESS 0x440000 // Alternate Xilinx configuration (Debug)
#define	RESET_ADDRESS	0x00480000 // ZipCPU Reset address

// Interrupt control constants
#define	GIE		0x80000000	// Enable all interrupts
#define	SCOPEN		0x80040004	// Enable WBSCOPE interrupts
#define	ISPIF_EN	0x88000800	// Enable SPI Flash interrupts
#define	ISPIF_DIS	0x08000000	// Disable SPI Flash interrupts
#define	ISPIF_CLR	0x08000800	// Clear pending SPI Flash interrupt

// Flash control constants
#define	ERASEFLAG	0x80000000
#define	DISABLEWP	0x10000000

// Sectors are defined as 64 kB (16 kW)
#define	SZPAGE		64	// 256 bytes
#define	PGLEN		64	// 256 bytes
#define	NPAGES		256	// 64 kB sectors / 256 bytes is ...
#define	SECTORSZ	(NPAGES * SZPAGE)
#define	NSECTORS	(FLASHWORDS/SECTORSZ)	// 256 sectors
#define	SECTOROF(A)	((A) & (-1<<14))	// 64 kB ea
#define	PAGEOF(A)	((A) & (-1<<6))

// Scop definition/sequences
#define	SCOPE_NO_RESET	0x80000000
#define	SCOPE_TRIGGER	(0x08000000|SCOPE_NO_RESET)
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
