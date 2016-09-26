//
//
// Filename:	uartsim.h
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

class	UARTSIM {
private:
	int	m_tx_busy_count, m_baud_counts, m_rx_busy_count;
	int	m_fdin, m_fdout;
	char	m_rx_next, m_tx_data;

public:
	UARTSIM(int baud_counts, int fdin = STDIN_FILENO,
			int fdout=STDOUT_FILENO);
	int	rx(unsigned char &data);
	int	tx(int stb, char data);
};


