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

DEPPI::DEPPI(char *szSel) {
	if (!DmgrOpen(&m_dev, szSel)) {
		fprintf(stderr, "Open failed!\n");
		exit(EXIT_FAILURE);
	}

	if (!DeppEnable(m_dev)) {
		fprintf(stderr, "Could not enable DEPP interface\n");
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
		char scode[cchErcMax], msg[cchErcMsgMax];
		DmgrSzFromErc(erc, scode, msg);
		fprintf(stderr, "ErrCode   : %s\n", scode);
		fprintf(stderr, "ErrMessage: %s\n", msg);
		close();
		exit(EXIT_FAILURE);
	}
}

void	DEPPI::write(char *buf, int len) {
	bool	good = true;

	if (false) {
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

	clock_gettime(CLOCK_MONOTONIC, &now);

	// printf("USBI::read(%d) (FIFO is %d-%d)\n", len, m_rend, m_rbeg);
	nr = pop_fifo(buf, left);
	left -= nr;
	
	while(left > 0) {
		raw_read(left, timeout_ms);
		nr = pop_fifo(&buf[len-left], left);
		left -= nr;

		// printf("\tWHILE (nr = %d, LEFT = %d, len=%d)\n", nr, left, len);
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

	// printf("READ %d characters (%d req, %d left)\n", len-left, len, left);
	return len-left;
}

void	DEPPI::raw_read(const int clen, int timeout_ms) {
	int	empty = RCV_BUFMASK - ((m_rbeg - m_rend)&(RCV_BUFMASK));
	int	len = clen;
	bool	good = true;

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
		// fprintf(stdout, "DEPP: Pushing to FIFO\n");
		push_fifo(m_rxbuf, ln);
		len -= ln;
	}

	if (!good)
		depperr();
}

void	DEPPI::flush_read(void) {
	while(poll(4)) {
		m_rbeg = m_rend = 0;
	}
}

void	DEPPI::push_fifo(char *buf, int len) {
	char	last = 0;
	char	*sptr = buf;

	// fprintf(stdout, "DEPP::PUSH(%d)\n", len);

	if (m_rbeg != m_rend)
		last = m_rbuf[(m_rbeg-1)&RCV_BUFMASK];
	for(int i=0; i<len; i++) {
		char v = *sptr++;
		if (((v & 0x80)||((unsigned char)v < 0x10))&&(v == last)) {
			// Skipp any stuff bytes
			// fprintf(stderr, "SKIPPING-1: %02x\n", v & 0x0ff);
		} else if ((unsigned char)v == 0x0ff) {
			// Skipp any not-yet-ready bytes
			// fprintf(stdout, "SKIPPING-2: %02x\n", 0x0ff);
		} else {
			m_rbuf[m_rbeg] = v;
			// fprintf(stdout, "PUSHING: %02x %c\n", v&0x0ff,
			//	isprint(v)?v:'.');
			m_rbeg = (m_rbeg+1)&(RCV_BUFMASK);
		} last = v;
	}
}

int	DEPPI::pop_fifo(char *buf, int len) {
	int	avail = (m_rbeg - m_rend)&(RCV_BUFMASK);
	int	left = len;
	int	nr = 0;

	// printf("Attempting to pop %d items from FIFO (%d - %d)\n",
	// 		len, m_rend, m_rbeg);
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

	// printf("POLL\n");
	if ((avail < 2)&&((avail<1)||(m_rbuf[m_rend]&0x80)||(m_rbuf[m_rend]<0x10))) {
		// printf("POLL -- CALLS RAW READ\n");
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
				// printf("POLL/LOOP -- %d available\n", avail);
			}
			if (avail < 1)
				r = false;
			else if ((avail==1)&&((m_rbuf[m_rend]&0x80)||(m_rbuf[m_rend]<0x10)))
				r = false;
		} else r = false;
	}
	// printf("POLL -- is %s\n", (r)?"true":"false");

	return r;
}
