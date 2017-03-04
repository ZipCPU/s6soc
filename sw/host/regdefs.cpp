////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	regdefs.cpp
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To give human readable names to the various registers available
//		internal to the processor on the wishbone bus.  This file is
//	primarily used for name to number translation within wbregs.cpp.
//	All names for a given register are equivalent, save only that the
//	register will always be identified by its first name in any output.
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
#include <strings.h>
#include <ctype.h>
#include "regdefs.h"

const	REGNAME	raw_bregs[] = {
	{ R_VERSION,	"VERSION"		},
	{ R_ICONTROL,	"ICONTROL"		},
	{ R_ICONTROL,	"INT"			},
	{ R_ICONTROL,	"PIC"			},
	{ R_ICONTROL,	"INTC"			},
	{ R_BUSERR,	"BUSERR"		},
	{ R_BUSERR,	"BUS"			},
	{ R_ITIMERA,	"TMRA"			},
	{ R_ITIMERB,	"TMRB"			},
	{ R_PWM,	"PWMAUDIO"		},
	{ R_PWM,	"PWM"			},
	{ R_PWM,	"PWMDATA"		},
	{ R_PWM,	"AUDIO"			},
	{ R_SPIO,	"SPIO"			},
	{ R_GPIO,	"GPIO"			},
	{ R_UART,	"UART"			},
	{ R_UART,	"UART-RX"		},
	{ R_UART,	"UARTRX"		},
	{ R_UART,	"RX"			},
	{ R_UART,	"UART-TX"		},
	{ R_UART,	"UARTTX"		},
	{ R_UART,	"TX"			},
	//
	{ R_QSPI_EREG,	"QSPIEREG"		},
	{ R_QSPI_EREG,	"QSPIE"			},
	{ R_QSPI_CREG,	"QSPICONF"		},
	{ R_QSPI_CREG,	"QSPIC"			},
	{ R_QSPI_SREG,	"QSPISTAT"		},
	{ R_QSPI_SREG,	"QSPIS"			},
	{ R_QSPI_IDREG, "QSPIID"		},
	{ R_QSPI_IDREG, "QSPII"			},
	//
	// The real-time-clock registers have been removed
	//	(they never fit in the core)
	//
	// The real-time-date register has been removed
	//	(it never fit either)
	//
	// The wishbone scope
	//
	{ R_SCOPE,	"SCOPE"			},
	{ R_SCOPE,	"SCOP"			},
	{ R_SCOPED,	"SCOPDATA"		},
	{ R_SCOPED,	"SCDATA"		},
	{ R_SCOPED,	"SCOPED"		},
	{ R_SCOPED,	"SCOPD"			},
	//
	// The ICAPE interface registers have been removed.
	//
	{ RAMBASE,	"MEM"			},
	{ SPIFLASH,	"FLASH"			}
};

#define	RAW_NREGS	(sizeof(raw_bregs)/sizeof(bregs[0]))

const	REGNAME	*bregs = raw_bregs;
const	int	NREGS = RAW_NREGS;

unsigned	addrdecode(const char *v) {
	if (isalpha(v[0])) {
		for(int i=0; i<NREGS; i++)
			if (strcasecmp(v, bregs[i].m_name)==0)
				return bregs[i].m_addr;
		fprintf(stderr, "Unknown register: %s\n", v);
		exit(-2);
	} else
		return strtoul(v, NULL, 0); 
}

const	char *addrname(const unsigned v) {
	for(int i=0; i<NREGS; i++)
		if (bregs[i].m_addr == v)
			return bregs[i].m_name;
	return NULL;
}

