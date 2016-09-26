////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	deppbus.cpp
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	This is a *very* simple Depp to Wishbone driver conversion.
//		Look in devbus.cpp for a description of how to use the driver.
//
//	The driver is simple: there are 9 registers of interest to run this
//	driver.  The first four registers, 0-3, are address registers, MSB
//	first.  Place your 32-bit address into these registers.  The next four
//	registers, 4-7, are data registers.  If writing data, place the data
//	to write into these registers.  The last register, 16, is a strobe
//	register.  Write a 1 to read, and a 3 to write, to this register.  A
// 	bus transaction will then take place.  Once completed, registers 4-7
//	will contain the resulting data.
//
//	That's the internal workings of this driver.  The above description is
//	accomplished in the readio() and writeio() routines.
//
//	This is *not* a fully featured DEVBUS class--it does not support
//	pipelined reads or writes.  It does not support compression.  It does,
//	however, support reading and writing a simple 32-bit wishbone bus.
//	That is good enough, of itself, for now.
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
#include <stdlib.h>
#include <stdio.h>
#include "dpcdecl.h"
#include "dmgr.h"
#include "depp.h"
#include "deppbus.h"

DEPPBUS::DEPPBUS(char *szSel) {
	if (!DmgrOpen(&m_dev, szSel)) {
		fprintf(stderr, "Open failed!\n");
		exit(EXIT_FAILURE);
	}

	if (!DeppEnable(m_dev)) {
		fprintf(stderr, "Could not enable DEPP interface\n");
		exit(EXIT_FAILURE);
	}

	m_int = false, m_err = false;
}

DEPPBUS::~DEPPBUS(void) {
	if (m_dev)
		DmgrClose(m_dev);
	m_dev = 0;
}

void DEPPBUS::kill(void) { close(); }
void DEPPBUS::close(void) { DmgrClose(m_dev); m_dev = 0; }

void	DEPPBUS::depperr(void) {
	ERC	erc = DmgrGetLastError();
	if (erc != ercNoErc) {
		char	scode[cchErcMax], msg[cchErcMsgMax];
		DmgrSzFromErc(erc,scode,msg);
		fprintf(stderr, "ErrCode   : %s\n", scode);
		fprintf(stderr, "ErrMessage: %s\n", msg);
		close();
		exit(EXIT_FAILURE);
	}
}

void DEPPBUS::writeio(const BUSW a, const BUSW v) {
	bool	good = true;

	// Set the address for our data
	good = good && DeppPutReg(m_dev, 0, (a>>24)&0x0ff, false);
	good = good && DeppPutReg(m_dev, 1, (a>>16)&0x0ff, false);
	good = good && DeppPutReg(m_dev, 2, (a>> 8)&0x0ff, false);
	good = good && DeppPutReg(m_dev, 3,  a     &0x0ff, false);
	if (!good) {
		fprintf(stderr, "BUS CYCLE FAILED\n");
		depperr(); close();
		exit(EXIT_FAILURE);
	}

	// Set the data to be transmitted
	good = good && DeppPutReg(m_dev, 4, (v>>24)&0x0ff, false);
	good = good && DeppPutReg(m_dev, 5, (v>>16)&0x0ff, false);
	good = good && DeppPutReg(m_dev, 6, (v>> 8)&0x0ff, false);
	good = good && DeppPutReg(m_dev, 7,  v     &0x0ff, false);
	if (!good) {
		fprintf(stderr, "BUS CYCLE FAILED\n");
		depperr(); close();
		exit(EXIT_FAILURE);
	}

	// Perform the operation
	good = good && DeppPutReg(m_dev,16,  0x3, false);
	if (!good) {
		fprintf(stderr, "BUS CYCLE FAILED\n");
		depperr(); close();
		exit(EXIT_FAILURE);
	}

	// Now, let's check for any bus errors and/or interrupts
	BYTE	retn;
	good = good && DeppGetReg(m_dev,16, &retn, false);
	m_err = m_err | (retn&1);
	m_int = m_int | (retn&2);

	if (!good) {
		fprintf(stderr, "BUS CYCLE FAILED\n");
		depperr(); close();
		exit(EXIT_FAILURE);
	}

	if (m_err)
		throw BUSERR(a);
}

DEVBUS::BUSW DEPPBUS::readio(const DEVBUS::BUSW a) {
	BUSW	v = 0;
	BYTE	retn;
	bool	good = true;

	// Set the address for our data
	good = good && DeppPutReg(m_dev, 0, (a>>24)&0x0ff, false);
	good = good && DeppPutReg(m_dev, 1, (a>>16)&0x0ff, false);
	good = good && DeppPutReg(m_dev, 2, (a>> 8)&0x0ff, false);
	good = good && DeppPutReg(m_dev, 3,  a     &0x0ff, false);

	if (!good) {
		fprintf(stderr, "BUS CYCLE FAILED\n");
		depperr(); close();
		exit(EXIT_FAILURE);
	}

	// Run the bus cycle
	good = good && DeppPutReg(m_dev,16,  0x1, false);
	if (!good) {
		fprintf(stderr, "BUS CYCLE FAILED\n");
		depperr(); close();
		exit(EXIT_FAILURE);
	}

	// Check for any bus errors and/or interrupts
	good = good && DeppGetReg(m_dev,16, &retn, false);
	if (!good) {
		fprintf(stderr, "BUS CYCLE FAILED\n");
		depperr(); close();
		exit(EXIT_FAILURE);
	}

	m_err = m_err | (retn&1);
	m_int = m_int | (retn&2);

	if (m_err)
		throw BUSERR(a);

	// Otherwise let's get our result
	good = good && DeppGetReg(m_dev, 4, &retn, false); v = (retn & 0x0ff);
	good = good && DeppGetReg(m_dev, 5, &retn, false); v = (v<<8)|(retn & 0x0ff);
	good = good && DeppGetReg(m_dev, 6, &retn, false); v = (v<<8)|(retn & 0x0ff);
	good = good && DeppGetReg(m_dev, 7, &retn, false); v = (v<<8)|(retn & 0x0ff);
	if (!good) {
		fprintf(stderr, "BUS CYCLE FAILED\n");
		depperr(); close();
		exit(EXIT_FAILURE);
	}

	return v;
}

void	DEPPBUS::readi(const BUSW a, const int len, BUSW *buf) {
	for(int i=0; i<len; i++)
		buf[i] = readio(a+i);
} void	DEPPBUS::readz(const BUSW a, const int len, BUSW *buf) {
	for(int i=0; i<len; i++)
		buf[i] = readio(a);
}

void	DEPPBUS::writei(const BUSW a, const int len, const BUSW *buf) {
	for(int i=0; i<len; i++)
		writeio(a+i, buf[i]);
} void	DEPPBUS::writez(const BUSW a, const int len, const BUSW *buf) {
	for(int i=0; i<len; i++)
		writeio(a, buf[i]);
}

bool	DEPPBUS::poll(void) {
	if (m_int)
		return true;
	
	// Check for any bus errors and/or interrupts
	BYTE	retn;
	DeppGetReg(m_dev,16, &retn, false);
	m_err = m_err | (retn&1);
	m_int = m_int | (retn&2);

	if (m_int)
		return true;
	return false;
} void	DEPPBUS::usleep(unsigned msec) {
	if (!poll())
		usleep(msec);
} void	DEPPBUS::wait(void) {
	while(!poll())
		usleep(5);
} bool	DEPPBUS::bus_err(void) const {
	return m_err;
} void	DEPPBUS::reset_err(void) {
	m_err = false;
} void	DEPPBUS::clear(void) {
	m_int = false;
	m_err = false;
}

