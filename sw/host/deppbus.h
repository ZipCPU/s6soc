////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	deppbus.h
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	An instantiation of a generic interface to a wishbone on a
//		device.  This particular instantiation uses a simplified
//	interface over a Digilent Adept Asynchronous Parallel Port Interface
//	(DEPP)--useful for working with the CMod-S6.
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
#ifndef	DEPPBUS_H
#define	DEPPBUS_H

#include "dpcdecl.h"
#include "dmgr.h"
#include "devbus.h"

class	DEPPBUS : public DEVBUS {
	HIF	m_dev;
	bool	m_int, m_err;

	void	depperr(void);

public:
	typedef	DEVBUS::BUSW BUSW;

	DEPPBUS(char *szSel);
	~DEPPBUS(void);

	void	kill(void);
	void	close(void);

	void	writeio(const BUSW a, const BUSW v);
	BUSW	readio(const BUSW a);

	void	readi(const BUSW a, const int len, BUSW *buf);
	void	readz(const BUSW a, const int len, BUSW *buf);

	void	writei(const BUSW a, const int len, const BUSW *buf);
	void	writez(const BUSW a, const int len, const BUSW *buf);
	bool	poll(void);
	void	usleep(unsigned msec);
	void	wait(void);
	bool	bus_err(void) const;
	void	reset_err(void);
	void	clear(void);
};

#endif
