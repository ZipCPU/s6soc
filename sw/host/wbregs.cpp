////////////////////////////////////////////////////////////////////////////////
//
// Filename:	wbregs.cpp
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To give a user access, via a command line program, to read
//		and write wishbone registers one at a time.  Thus this program
//		implements readio() and writeio() but nothing more.
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

int main(int argc, char **argv) {
	int	skp=0;

	skp=1;
	for(int argn=0; argn<argc-skp; argn++) {
		if (argv[argn+skp][0] == '-') {
			// switch(argv[argn+skp][j]) {
			// default:
				// break;
			// }
			skp++; argn--;
		} else
			argv[argn] = argv[argn+skp];
	} argc -= skp;

	signal(SIGSTOP, closeup);
	signal(SIGHUP, closeup);

	if ((argc < 1)||(argc > 2)) {
		// usage();
		printf("USAGE: wbregs address [value]\n");
		exit(-1);
	}

	const char *nm;
	unsigned address = addrdecode(argv[0]), value;
	nm = addrname(address);
	if (nm == NULL)
		nm = "no name";

	char	szSel[64];
	strcpy(szSel, S6SN);
	m_fpga = FPGAOPEN(szSel);

	if (argc < 2) {
		FPGA::BUSW	v;
		try {
			unsigned char a, b, c, d;
			v = m_fpga->readio(address);
			a = (v>>24)&0x0ff;
			b = (v>>16)&0x0ff;
			c = (v>> 8)&0x0ff;
			d = (v    )&0x0ff;
			printf("%08x (%8s) : [%c%c%c%c] %08x\n", address, nm, 
				isgraph(a)?a:'.', isgraph(b)?b:'.',
				isgraph(c)?c:'.', isgraph(d)?d:'.', v);
		} catch(BUSERR b) {
			printf("%08x (%8s) : BUS-ERROR\n", address, nm);
		}
	} else {
		value = strtoul(argv[1], NULL, 0);
		m_fpga->writeio(address, value);
		printf("%08x (%8s)-> %08x\n", address, nm, value);
	}

	if (m_fpga->poll())
		printf("FPGA was interrupted\n");
	delete	m_fpga;
}

