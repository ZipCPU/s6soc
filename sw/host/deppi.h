////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	deppi.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	This package attempts to convet a DEPP over USB based
//		communication system into something similar to a serial port
//	based communication system.  Some differences include the fact that,
//	if the DEPP port isn't polled, nothing comes out of the port.  Hence,
//	on connecting (or polling for the first time) ... there might be a 
//	bunch of stuff to (initially) ignore.
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
#ifndef	DEPPI_H
#define	DEPPI_H

#include "dpcdecl.h"
#include "dmgr.h"
// #include "devbus.h"

//
#define	PKTLEN	32
#define	RCV_BUFLEN	512
#define	RCV_BUFMASK	(RCV_BUFLEN-1)

// #define	S6SN	"SN:210282768825"
#define	S6SN	""

#include "llcomms.h"

class	DEPPI : public LLCOMMSI { // DEPP Interface
private:
	HIF	m_dev;
	bool	m_int, m_err;
	char	m_rbuf[RCV_BUFLEN];
	char	m_txbuf[2*PKTLEN], m_rxbuf[2*PKTLEN];
	int	m_rbeg, m_rend;

	virtual	int	pop_fifo(char *buf, int len);
	virtual	void	push_fifo(char *buf, int len);
	virtual	void	raw_read(int len, int timeout_ms);
	virtual	void	flush_read(void);
	void	depperr(void);

public:
	DEPPI(const char *szSel);
	~DEPPI(void);

	virtual	void	close(void);
	virtual	int	read(char *buf, int len);
	virtual	int	read(char *buf, int len, int timeout_ms);
	virtual	void	write(char *buf, int len);
	virtual	bool	poll(unsigned ms);
};

#endif // DEPPI_H

