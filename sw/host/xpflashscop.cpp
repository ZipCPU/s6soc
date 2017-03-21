////////////////////////////////////////////////////////////////////////////////
//
// Filename:	xpflashscop.cpp
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To test the QSPI Xpress flash interface, to see what it is doing
//		on the chip, and to create a VCD file matching the wires
//	determined/seen from the chip.
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
//
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include "devbus.h"

#ifdef	SIMPLE_DEPPBUS
# include "deppbus.h"
#else
# include "llcomms.h"
# include "deppi.h"
# include "ttybus.h"

#endif

// #include "port.h"
#include "regdefs.h"
#include "scopecls.h"

#ifdef	SIMPLE_DEPPBUS
# define	FPGAOPEN(SN)	new DEPPBUS(SN);
#else
# define	FPGAOPEN(SN)	new FPGA(new DEPPI(SN))
#endif

FPGA	*m_fpga;
void	closeup(int v) {
	m_fpga->kill();
	exit(0);
}

class	XPFLASHSCOPE : public SCOPE {
public:
	XPFLASHSCOPE(DEVBUS *fpga) : SCOPE(fpga, R_SCOPE, false, true) {
		define_traces();
	}

	// Inside our design, we have recorded 32-bits of information describing
	// what's going on.  Here, we describe the names and widths of the
	// components of those 32-bits.  These are used to create a VCD file
	// that can be loaded into GTKWave if necessary to see what's going on.
	virtual	void	define_traces(void) {
		register_trace("wb_cyc",      1, 31);
		register_trace("wb_stb",      1, 30);
		register_trace("flash_sel",   1, 29);
		register_trace("flctl_sel",   1, 28);
		register_trace("flash_ack",   1, 27);
		register_trace("flash_stall", 1, 26);
		register_trace("qspi_cs_n",   1, 25); // blank/unused bit next
		register_trace("qspi_sck",    2, 23);
		register_trace("qspi_mod",    2, 21);
		register_trace("o_qspi_dat",  4, 16);
		register_trace("i_qspi_dat",  4, 12);
		register_trace("flash_data", 12,  0);
	}

	// In case you aren't interested in creating a VCD file, we create an
	// output decoder, allowing you to "see" (textually) the output.  This
	// is the default, although it may be ignored.
	virtual	void	decode(DEVBUS::BUSW v) const {
		printf("%3s%3s %3s%3s %3s%5s",
			((v>>31)&1)?"CYC":"",
			((v>>30)&1)?"STB":"",
			((v>>29)&1)?"SEL":"",
			((v>>28)&1)?"CTL":"",
			((v>>27)&1)?"ACK":"",
			((v>>26)&1)?"STALL":"");
		printf("  %2s[%d%d,%d] %x->%x ",
			((v>>25)&1)?"":"CS",
			((v>>24)&1), ((v>>23)&1),
			((v>>21)&3), // Mode
			((v>>16)&0xf),
			((v>>12)&0xf));

		printf("%03x", (v&0x0fff));
	}
};

int main(int argc, char **argv) {
	char	szSel[64];

	// First step: connect to our FPGA.
	strcpy(szSel, S6SN);
	m_fpga = FPGAOPEN(szSel);

	// Second step: create a piece of software to read the scope off of the
	// FPGA device, and to decode it later.
	XPFLASHSCOPE	*xpscope = new XPFLASHSCOPE(m_fpga);

	// Check if the scope is "ready".  It will be "ready" when the scope
	// has been both primed, triggered, and stopped.  If the scope hasn't
	// triggered, or hasn't stopped recording, you'll need to come back
	// later.
	if (xpscope->ready()) {
		// The scope is ready.  Hence, it has a buffer filled with
		// the data we are looking for.

		// Read the data off of the scope, and print it to the screen
		xpscope->print();

		// Should you wish to, you may also create a .VCD file with the
		// scopes outputs within it.
		xpscope->writevcd("xpscope.vcd");
	} else {
		//
		// If the scope wasn't ready (yet) to be read, lets output
		// the state of the scope, so that we can tell what happened
		// or more specifically, what hasn't happened.  (For example,
		// if the trigger hasn't gone off, then we need to wait longer
		// for it to go off.
		//
		xpscope->decode_control();
	}

	delete	m_fpga;
}

