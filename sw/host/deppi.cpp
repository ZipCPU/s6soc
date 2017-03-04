////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	deppi.cpp
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	Creates a DEPP interface pseudo-character device, similar to
//		what you might get from a serial port or other character device,
//	from the DEPP interface to a CMOD S6 board.
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
// From Digilent's Adept Library
#include "dpcdecl.h"
#include "dmgr.h"
#include "depp.h"

// From my own library
#include "llcomms.h"
#include "deppi.h"

FILE	*dbgfp = stderr;

DEPPI::DEPPI(const char *szSel) {
	if ((!szSel)||(szSel[0] == '\0')) {
		// Number of digilent devcies on a system
		int	pcdvc;

		// Go fish and try to find the device
		DmgrEnumDevices(&pcdvc);

		if (pcdvc < 0) {
			depperr();
			exit(EXIT_FAILURE);
		}

		//
		int	found = 0; // Number of devices found mtg our criteria
		DVC	dvcinfo; // A structure to receive device info
		int	foundid=-1; // The id number of the device we found

		//
		for(int devid=0; devid < pcdvc; devid++) {
			DmgrGetDvc(devid, &dvcinfo);
			// fprintf(dbgfp, "DEVICE NAME: %s\n", dvcinfo.szName);
			if (strcmp(dvcinfo.szName, "CmodS6")==0) {
				found++;
				// fprintf(dbgfp, "Found a CMOD!\n");
				foundid = devid;
			}
		}

		if (found == 0) {
			fprintf(stderr, "No CModS6 devices found\n");
			exit(EXIT_FAILURE);
		} else if (found > 1) {
			fprintf(stderr, "More than one CModS6 device found.  Please consider opening your\n");
			fprintf(stderr, "device with a valid serial number instead.\n");
			exit(EXIT_FAILURE);
		}

		DmgrGetDvc(foundid, &dvcinfo);
		if (!DmgrOpen(&m_dev, dvcinfo.szConn)) {
			fprintf(stderr, "Could not open device!\n");
			depperr();
			exit(EXIT_FAILURE);
		}

		//
		DmgrFreeDvcEnum();
	} else if (!DmgrOpen(&m_dev, (char *)szSel)) {
		// We know the device serial number, so go open that particular
		// device
		fprintf(stderr, "Named device open (DmgrOpen) failed!\n");
		depperr();
		exit(EXIT_FAILURE);
	}

	if (!DeppEnable(m_dev)) {
		fprintf(stderr, "Could not enable DEPP interface to (opened) device\n");

		depperr();
		exit(EXIT_FAILURE);
	}

	m_int = false, m_err = false;

	// fprintf(stdout, "Flushing **************\n");
	flush_read();
	// fprintf(stdout, "Flushed! **************\n");
}

DEPPI::~DEPPI(void) {
	close();
}

void	DEPPI::close(void) {
	if (m_dev)
		DmgrClose(m_dev);
	m_dev = 0;
}

void	DEPPI::depperr(void) {
	ERC	erc = DmgrGetLastError();
	if(erc != ercNoErc) {
		char scode[cchErcMax], smsg[cchErcMsgMax];
		DmgrSzFromErc(erc, scode, smsg);
		fprintf(stderr, "ErrCode(%d): %s\n", erc, scode);
		fprintf(stderr, "ErrMessage: %s\n", smsg);

		if (erc == ercCapabilityConflict) {
			fprintf(stderr, "Do you have the hardware manager in Vivado open?\n");
			fprintf(stderr, "That could cause this conflict.\n");
		}
		close();
		exit(EXIT_FAILURE);
	}
}

void	DEPPI::write(char *buf, int len) {
	bool	good = true;
	const bool	dbg = false;

	if (dbg) {
		// Debug code--write one at a time
		fputs("WR: ", stdout);
		for(int i=0; i<len; i++) {
			good = good && DeppPutReg(m_dev, 0, (unsigned char)buf[i], false);
			fputc(buf[i], stdout);
		} fputc('\n', stdout);
	} else
		good = DeppPutRegRepeat(m_dev, 0, (unsigned char *)buf, len, false);
	if (!good)
		depperr();
}

int	DEPPI::read(char *buf, int len) {
	return read(buf, len, 4);
}

int	DEPPI::read(char *buf, int len, int timeout_ms) {
	int	left = len, nr=0;
	struct	timespec	now, later;
	const	bool	dbg = false;

	clock_gettime(CLOCK_MONOTONIC, &now);

	if (dbg) fprintf(dbgfp, "USBI::read(%d) (FIFO is %d-%d)\n", len, m_rend, m_rbeg);
	nr = pop_fifo(buf, left);
	left -= nr;
	
	while(left > 0) {
		raw_read(left, timeout_ms);
		nr = pop_fifo(&buf[len-left], left);
		left -= nr;

		if (dbg) fprintf(dbgfp, "\tWHILE (nr = %d, LEFT = %d, len=%d)\n", nr, left, len);
		if (nr == 0)
			break;
#define	TIMEOUT
#ifdef	TIMEOUT
		if (timeout_ms == 0)
			break;
		else if (timeout_ms > 0) {
			clock_gettime(CLOCK_MONOTONIC, &later);

			long	num_ns = later.tv_nsec - now.tv_nsec, num_ms;
			if (num_ns < 0) {
				num_ns += 1000000000;
				later.tv_sec--;
			} num_ms = num_ns / 1000000;
			if (later.tv_sec > now.tv_sec)
				num_ms += (later.tv_sec - now.tv_sec)*1000;

			if (num_ms > timeout_ms)
				break;
		}
#endif
	}

	if(dbg) fprintf(dbgfp, "READ %d characters (%d req, %d left)\n", len-left, len, left);
	return len-left;
}

void	DEPPI::raw_read(const int clen, int timeout_ms) {
	int	empty = RCV_BUFMASK - ((m_rbeg - m_rend)&(RCV_BUFMASK));
	int	len = clen;
	bool	good = true;
	const	bool	dbg = false;


	if (dbg) fprintf(dbgfp, "DEPPI::raw_read(len=%d)\n", clen);
	if (len > empty)
		len = empty;
	if (len > 0) {
		// Fill the tail of our buffer
		int ln = len;
		if (ln > PKTLEN)
			ln = PKTLEN;

		// fprintf(stdout, "RAW-READ(%d)\n", ln);
		if (false) {
			// Debug code--read one word at a time
			for(int i=0; i<ln; i++) {
				good = good && DeppGetReg(m_dev, 0, (unsigned char *)&m_rxbuf[i], false);
				usleep(1);
			}
		} else
			good = good && DeppGetRegRepeat(m_dev, 0, (unsigned char *)m_rxbuf, ln, false);
		if(dbg) fprintf(dbgfp, "DEPP: Pushing to FIFO\n");
		push_fifo(m_rxbuf, ln);
		len -= ln;
	}

	if (!good)
		depperr();
}

void	DEPPI::flush_read(void) {
	const	bool	dbg = false;

	if (dbg)	fprintf(dbgfp, "DEPPI::FLUSH-READ()\n");

	do {
		m_rbeg = m_rend = 0;
	} while(poll(4));

	if (dbg)	fprintf(dbgfp, "DEPPI::FLUSH-READ() -- COMPLETE\n");
}

void	DEPPI::push_fifo(char *buf, int len) {
	char	last = 0;
	char	*sptr = buf;
	const	bool	dbg = false;

	if (dbg)  fprintf(dbgfp, "DEPP::PUSH(%d)\n", len);

	if (m_rbeg != m_rend)
		last = m_rbuf[(m_rbeg-1)&RCV_BUFMASK];
	if (dbg)	fprintf(dbgfp, "DEPPI::PUSH() last=%d, rbeg=%d, rend=%d\n", last, m_rbeg, m_rend);
	for(int i=0; i<len; i++) {
		char v = *sptr++;
		if (((v & 0x80)||((unsigned char)v < 0x10))&&(v == last)) {
			// Skipp any stuff bytes
			if (dbg)  fprintf(dbgfp, "SKIPPING-1: %02x\n", v & 0x0ff);
		} else if ((unsigned char)v == 0x0ff) {
			// Skipp any not-yet-ready bytes
			if (dbg)  fprintf(dbgfp, "SKIPPING-2: %02x\n", 0x0ff);
		} else {
			m_rbuf[m_rbeg] = v;
			if (dbg) fprintf(dbgfp, "PUSHING: 0x%02x \'%c\'\n",
				v&0x0ff, isprint(v)?v:'.');
			m_rbeg = (m_rbeg+1)&(RCV_BUFMASK);
		} last = v;
	}
}

int	DEPPI::pop_fifo(char *buf, int len) {
	int	avail = (m_rbeg - m_rend)&(RCV_BUFMASK);
	int	left = len;
	int	nr = 0;
	const	bool	dbg = false;

	if (dbg) fprintf(dbgfp, "Attempting to pop %d items from FIFO (%d - %d)\n",
	 		len, m_rend, m_rbeg);
	while((avail > 0)&&(left > 0)) {
		int ln = RCV_BUFLEN-m_rend;
		if (ln > left)
			ln = left;
		if (ln > avail)
			ln = avail;
		memcpy(&buf[len-left], &m_rbuf[m_rend], ln);
		left   -= ln;
		avail  -= ln;
		m_rend  = (m_rend + ln)&(RCV_BUFMASK);
		nr     += ln;

	}

	return nr;
}

bool	DEPPI::poll(unsigned ms) {
	int	avail = (m_rbeg-m_rend)&(RCV_BUFMASK);
	bool	r = true;
	const	bool	dbg = false;

	if (dbg) fprintf(dbgfp, "POLL\n");
	if ((avail < 2)&&((avail<1)||(m_rbuf[m_rend]&0x80)||(m_rbuf[m_rend]<0x10))) {
		if (dbg) fprintf(dbgfp, "POLL -- CALLS RAW READ\n");
		raw_read(4,ms);
		avail = (m_rbeg-m_rend)&(RCV_BUFMASK);

		if (avail != 0) {
			// Read 'til there's nothing more to be read
			char	v = (m_rbuf[(m_rbeg-1)&(RCV_BUFMASK)]);
			while(((v&0x80)==0)&&((unsigned)v>=0x10)&&(avail < RCV_BUFMASK-32)) {
				raw_read(26,ms);
				if (avail == ((m_rbeg-m_rend)&(RCV_BUFMASK)))
					break; // We didn't read anything more
				avail = (m_rbeg-m_rend)&(RCV_BUFMASK);
				if (dbg) fprintf(dbgfp, "POLL/LOOP -- %d available\n", avail);
			}
			if (avail < 1)
				r = false;
			else if ((avail==1)&&((m_rbuf[m_rend]&0x80)||(m_rbuf[m_rend]<0x10)))
				r = false;
		} else r = false;
	}
	if (dbg) fprintf(dbgfp, "POLL -- is %s\n", (r)?"true":"false");

	return r;
}
