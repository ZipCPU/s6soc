//
//
// Filename:	uartsim.cpp
//
// Project:	FPGA library development (S6 development board)
//
// Purpose:	To emulate the external parameters of a UART device, providing
//		the UART output to, and input from, the command
//		console/terminal while also providing the controller with
//		appropriate busy lines as necessary.
//
// Creator:	Dan Gisselquist
//		Gisselquist Tecnology, LLC
//
// Copyright:	2016
//
//
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "uartsim.h"

UARTSIM::UARTSIM(int baud_counts, int fdin, int fdout) {
	int fctl_flags;

	m_fdin = fdin;
	m_fdout = fdout;
	m_baud_counts = baud_counts;

	m_tx_busy_count = 0;
	m_rx_busy_count = -1;

	// Set the linux O_NONBLOCK on fdin
	fctl_flags = fcntl(fdin, F_GETFL, 0);
	fcntl(fdin, F_SETFL, fctl_flags | O_NONBLOCK);
}

int	UARTSIM::rx(unsigned char &data) {
	if (m_rx_busy_count >= 0) {
		if (m_rx_busy_count-- == 0) {
			data = (unsigned char)m_rx_next;
			return 1;
		}
	}

	if (read(m_fdin, &m_rx_next, 1) > 0)
		m_rx_busy_count = m_baud_counts;

	return 0;
}

int	UARTSIM::tx(int stb, char data) {
	if (m_tx_busy_count > 0) {

		// This write may block--if so, we don't care.  Just write it.
		if (--m_tx_busy_count == 0)
			if (write(m_fdout, &m_tx_data, 1) != 1)
				perror("O/S Write Err:");
		return 1;
	} else if (stb) {
		m_tx_data = data;
		m_tx_busy_count = m_baud_counts;
	}

	return 0;
}


