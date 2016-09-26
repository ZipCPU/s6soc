////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	flashdrvr.cpp
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	Flash driver.  Encapsulate writing to the flash device.
//
// Creator:	Dan Gisselquist
//		Gisselquist Tecnology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016, Gisselquist Technology, LLC
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
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
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
#include "regdefs.h"
#include "flashdrvr.h"

const	bool	HIGH_SPEED = false;

void	FLASHDRVR::flwait(void) {
	DEVBUS::BUSW	v;

	v = m_fpga->readio(R_QSPI_EREG);
	if ((v&ERASEFLAG)==0)
		return;
	m_fpga->writeio(R_ICONTROL, ISPIF_DIS);
	m_fpga->clear();
	m_fpga->writeio(R_ICONTROL, ISPIF_EN);

	do {
		// Start by checking that we are still erasing.  The interrupt
		// may have been generated while we were setting things up and
		// disabling things, so this just double checks for us.  If
		// the interrupt was tripped, we're done.  If not, we can now
		// wait for an interrupt.
		v = m_fpga->readio(R_QSPI_EREG);
		if (v&ERASEFLAG) {
			m_fpga->usleep(400);
			if (m_fpga->poll()) {
				m_fpga->clear();
				m_fpga->writeio(R_ICONTROL, ISPIF_EN);
			}
		}
	} while(v & ERASEFLAG);
}

bool	FLASHDRVR::erase_sector(const unsigned sector, const bool verify_erase) {
	DEVBUS::BUSW	page[SZPAGE];

	printf("Erasing sector: %08x\n", sector);
	m_fpga->writeio(R_QSPI_EREG, DISABLEWP);
	m_fpga->writeio(R_QSPI_EREG, ERASEFLAG + sector);

	// If we're in high speed mode and we want to verify the erase, then
	// we can skip waiting for the erase to complete by issueing a read
	// command immediately.  As soon as the erase completes the read will
	// begin sending commands back.  This allows us to recover the lost 
	// time between the interrupt and the next command being received.
	if  ((!HIGH_SPEED)||(!verify_erase)) {
		flwait();

		printf("@%08x -> %08x\n", R_QSPI_EREG,
				m_fpga->readio(R_QSPI_EREG));
		printf("@%08x -> %08x\n", R_QSPI_SREG,
				m_fpga->readio(R_QSPI_SREG));
		printf("@%08x -> %08x\n", sector,
				m_fpga->readio(sector));
	}

	// Now, let's verify that we erased the sector properly
	if (verify_erase) {
		for(int i=0; i<NPAGES; i++) {
			m_fpga->readi(sector+i*SZPAGE, SZPAGE, page);
			for(int i=0; i<SZPAGE; i++)
				if (page[i] != 0xffffffff)
					return false;
		}
	}

	return true;
}

bool	FLASHDRVR::write_page(const unsigned addr, const unsigned len,
		const unsigned *data, const bool verify_write) {
	DEVBUS::BUSW	buf[SZPAGE];

	assert(len > 0);
	assert(len <= PGLEN);
	assert(PAGEOF(addr)==PAGEOF(addr+len-1));

	if (len <= 0)
		return true;

	// Write the page
	m_fpga->writeio(R_ICONTROL, ISPIF_DIS);
	m_fpga->clear();
	m_fpga->writeio(R_ICONTROL, ISPIF_EN);
	printf("Writing page: 0x%08x - 0x%08x\n", addr, addr+len-1);
	m_fpga->writeio(R_QSPI_EREG, DISABLEWP);
	m_fpga->writei(addr, len, data);

	// If we're in high speed mode and we want to verify the write, then
	// we can skip waiting for the write to complete by issueing a read
	// command immediately.  As soon as the write completes the read will
	// begin sending commands back.  This allows us to recover the lost 
	// time between the interrupt and the next command being received.
	flwait();
	// if ((!HIGH_SPEED)||(!verify_write)) { }
	if (verify_write) {
		// printf("Attempting to verify page\n");
		// NOW VERIFY THE PAGE
		m_fpga->readi(addr, len, buf);
		for(unsigned i=0; i<len; i++) {
			if (buf[i] != data[i]) {
				printf("\nVERIFY FAILS[%d]: %08x\n", i, i+addr);
				printf("\t(Flash[%d]) %08x != %08x (Goal[%08x])\n", 
					i, buf[i], data[i], i+addr);
				return false;
			}
		} // printf("\nVerify success\n");
	} return true;
}

bool	FLASHDRVR::write(const unsigned addr, const unsigned len,
		const unsigned *data, const bool verify) {
	// Work through this one sector at a time.
	// If this buffer is equal to the sector value(s), go on
	// If not, erase the sector

	/*
	fprintf(stderr, "FLASH->write(%08x, %d, ..., %s)\n", addr, len,
			(verify)?"Verify":"");
	*/
	// m_fpga->writeio(R_QSPI_CREG, 2);
	// m_fpga->readio(R_VERSION);	// Read something innocuous
	// m_fpga->writeio(R_QSPI_SREG, 0);
	// m_fpga->readio(R_VERSION);	// Read something innocuous

	for(unsigned s=SECTOROF(addr); s<SECTOROF(addr+len+SECTORSZ-1); s+=SECTORSZ) {
		// Do we need to erase?
		bool	need_erase = false;
		unsigned newv = 0; // (s<addr)?addr:s;
		{
			DEVBUS::BUSW	*sbuf = new DEVBUS::BUSW[SECTORSZ];
			const DEVBUS::BUSW *dp;
			unsigned	base,ln;
			base = (addr>s)?addr:s;
			ln=((addr+len>s+SECTORSZ)?(s+SECTORSZ):(addr+len))-base;
			m_fpga->readi(base, ln, sbuf);

			dp = &data[base-addr];
			for(unsigned i=0; i<ln; i++) {
				if ((sbuf[i]&dp[i]) != dp[i]) {
					printf("\nNEED-ERASE @0x%08x ... %08x != %08x (Goal)\n", 
						i+base, sbuf[i], dp[i]);
					need_erase = true;
					newv = i+base;
					break;
				} else if ((sbuf[i] != dp[i])&&(newv == 0)) {
					// if (newv == 0)
						// printf("MEM[%08x] = %08x (!= %08x (Goal))\n",
							// i+base, sbuf[i], dp[i]);
					newv = i+base;
				}
			}
		}

		if (newv == 0)
			continue; // This sector already matches

		// Just erase anyway
		if (!need_erase)
			printf("NO ERASE NEEDED\n");
		else {
			printf("ERASING SECTOR: %08x\n", s);
			if (!erase_sector(s, verify)) {
				printf("SECTOR ERASE FAILED!\n");
				return false;
			} newv = (s<addr) ? addr : s;
		}
		for(unsigned p=newv; (p<s+SECTORSZ)&&(p<addr+len); p=PAGEOF(p+PGLEN)) {
			unsigned start = p, len = addr+len-start;

			// BUT! if we cross page boundaries, we need to clip
			// our results to the page boundary
			if (PAGEOF(start+len-1)!=PAGEOF(start))
				len = PAGEOF(start+PGLEN)-start;
			if (!write_page(start, len, &data[p-addr], verify)) {
				printf("WRITE-PAGE FAILED!\n");
				return false;
			}
		}
	}

	m_fpga->writeio(R_QSPI_EREG, 0); // Re-enable write protection

	return true;
}

