////////////////////////////////////////////////////////////////////////////////
//
// Filename:	zipload.cpp
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To load the flash--both a the two configurations and the 
//		a program for the ZipCPU into (flash) memory.
//
// 	Steps:
//		1. Reboot the CMod into the alternate/debug/command mode
//		2. Load flash memory
//		3. Reload (reboot) the CMod configuration into ZipCPU mode
//		4. Program should start on its own.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015-2017, Gisselquist Technology, LLC
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
// with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include "devbus.h"
#include "llcomms.h"
#include "deppi.h"
#include "regdefs.h"
#include "flashdrvr.h"
#include "zipelf.h"

FPGA	*m_fpga;

void	usage(void) {
	printf("USAGE: zipload [-h] [<bit-file> [<alt-bit-file>]] <zip-program-file>\n");
	printf("\n"
"\t-h\tDisplay this usage statement\n"
);
}

void	skip_bitfile_header(FILE *fp) {
	const unsigned	SEARCHLN = 204, MATCHLN = 16;
	const unsigned char matchstr[MATCHLN] = {
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		//
		0xaa, 0x99, 0x55, 0x66 };
	unsigned char	buf[SEARCHLN];

	rewind(fp);
	fread(buf, sizeof(char), SEARCHLN, fp);
	for(int start=0; start+MATCHLN<SEARCHLN; start++) {
		int	mloc;

		// Search backwards, since the starting bytes just aren't that
		// interesting.
		for(mloc = MATCHLN-1; mloc >= 0; mloc--)
			if (buf[start+mloc] != matchstr[mloc])
				break;
		if (mloc < 0) {
			fseek(fp, start, SEEK_SET);
			return;
		}
	}

	fprintf(stderr, "Could not find bin-file header within bit file\n");
	fclose(fp);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	int		skp=0, argn;
	bool		debug_only = false, verbose = false;
	bool		ignore_missing_memory = false;
	unsigned	entry = 0;
	FLASHDRVR	*flash = NULL;
	const char	*bitfile = NULL, *altbitfile = NULL, *execfile = NULL;
	size_t		bitsz;
	FILE		*fp;

	if (argc < 2) {
		usage();
		exit(EXIT_SUCCESS);
	}

	skp=1;
	for(argn=0; argn<argc-skp; argn++) {
		if (argv[argn+skp][0] == '-') {
			switch(argv[argn+skp][1]) {
			case 'd':
				debug_only = true;
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				verbose = true;
				break;
			default:
				fprintf(stderr, "Unknown option, -%c\n\n",
					argv[argn+skp][0]);
				usage();
				exit(EXIT_FAILURE);
				break;
			} skp++; argn--;
		} else {
			// Anything here must be either the program to load,
			// or a bit file to load
			argv[argn] = argv[argn+skp];
		}
	} argc -= skp;


	for(argn=0; argn<argc; argn++) {
		if (iself(argv[argn])) {
			if (execfile) {
				printf("Too many executable files given, %s and %s\n", execfile, argv[argn]);
				usage();
				exit(EXIT_FAILURE);
			} execfile = argv[argn];
		} else { // if (isbitfile(argv[argn]))
			if (!bitfile)
				bitfile = argv[argn];
			else if (!altbitfile)
				altbitfile = argv[argn];
			else {
				printf("Unknown file name or too many files, %s\n", argv[argn]);
				usage();
				exit(EXIT_FAILURE);
			}
		}
	}

if (verbose) {
if (bitfile)	printf(" BITFILE: %s\n", bitfile);
if (altbitfile)	printf("ABITFILE: %s\n", altbitfile);
if (execfile)	printf("EXECTFILE: %s\n", execfile);
}

	if ((execfile == NULL)&&(bitfile == NULL)) {
		printf("No executable or bit file(s) given!\n\n");
		usage();
		exit(EXIT_FAILURE);
	}

	if ((bitfile == NULL)&&(altbitfile != NULL)) {
		printf("Cannot program an alternate bitfile without a main bitfile\n\n");
		usage();
		exit(EXIT_FAILURE);
	}

	if ((bitfile)&&(access(bitfile,R_OK)!=0)) {
		// If there's no code file, or the code file cannot be opened
		fprintf(stderr, "Cannot open bitfile, %s\n", bitfile);
		exit(EXIT_FAILURE);
	}

	if ((altbitfile)&&(access(altbitfile,R_OK)!=0)) {
		// If there's no code file, or the code file cannot be opened
		fprintf(stderr, "Cannot open alternate bitfile, %s\n", altbitfile);
		exit(EXIT_FAILURE);
	} if ((execfile)&&(access(execfile,R_OK)!=0)) {
		// If there's no code file, or the code file cannot be opened
		fprintf(stderr, "Cannot open executable, %s\n\n", execfile);
		usage();
		exit(EXIT_FAILURE);
	} else if (!iself(execfile)) {
		printf("%s is not an executable file\n\n", execfile);
		usage();
		exit(EXIT_FAILURE);
	}

	char	*fbuf = new char[FLASHLEN];

	// Set the flash buffer to all ones
	memset(fbuf, -1, FLASHLEN);

	if (debug_only) {
		m_fpga = NULL;
	} else {
        	char    szSel[64];
        	strcpy(szSel, S6SN);
		m_fpga = new FPGA(new DEPPI(szSel));
	}

	// Make certain we can talk to the FPGA
	try {
		unsigned v  = m_fpga->readio(R_VERSION);
		if (v < 0x20170000) {
			fprintf(stderr, "Could not communicate with board (invalid version)\n");
			exit(EXIT_FAILURE);
		}
	} catch(BUSERR b) {
		fprintf(stderr, "Could not communicate with board (BUSERR when reading VERSION)\n");
		exit(EXIT_FAILURE);
	}

	flash = (debug_only)?NULL : new FLASHDRVR(m_fpga);

	// First, see if we need to load a bit file
	if (bitfile) {

		fp = fopen(bitfile, "r");
		if (strcmp(&argv[argn][strlen(argv[argn])-4],".bit")==0)
			skip_bitfile_header(fp);
		bitsz = fread(&fbuf[CONFIG_ADDRESS-SPIFLASH],
				sizeof(fbuf[0]),
				FLASHLEN - (CONFIG_ADDRESS-SPIFLASH), fp);
		fclose(fp);

		try {
			printf("Loading: %s\n", bitfile);
			flash->write(CONFIG_ADDRESS, bitsz, fbuf, true);
		} catch(BUSERR b) {
			fprintf(stderr, "BUS-ERR @0x%08x\n", b.addr);
			exit(-1);
		}
	}

	// Then see if we were given an alternate bit file
	if (altbitfile) {
		size_t	altsz;
		assert(CONFIG_ADDRESS + bitsz < ALTCONFIG_ADDRESS);

		fp = fopen(altbitfile, "r");
		if (strcmp(&argv[argn][strlen(argv[argn])-4],".bit")==0)
			skip_bitfile_header(fp);
		altsz = fread(&fbuf[ALTCONFIG_ADDRESS-SPIFLASH],
				sizeof(fbuf[0]),
				FLASHLEN-(ALTCONFIG_ADDRESS-SPIFLASH), fp);
		assert(ALTCONFIG_ADDRESS+altsz < RESET_ADDRESS);
		fclose(fp);

		try {
			printf("Loading: %s\n", altbitfile);
			flash->write(ALTCONFIG_ADDRESS, altsz, fbuf, true);
		} catch(BUSERR b) {
			fprintf(stderr, "BUS-ERR @0x%08x\n", b.addr);
			exit(-1);
		}
	} else {
		assert(CONFIG_ADDRESS+bitsz < RESET_ADDRESS);
	}

	if (execfile) try {
		ELFSECTION	**secpp = NULL, *secp;

		if(iself(execfile)) {
			// zip-readelf will help with both of these ...
			elfread(execfile, entry, secpp);
			assert(entry == RESET_ADDRESS);
		} else {
			fprintf(stderr, "ERR: %s is not in ELF format\n", execfile);
			exit(EXIT_FAILURE);
		}

		printf("Loading: %s\n", execfile);
		// assert(secpp[1]->m_len = 0);
		for(int i=0; secpp[i]->m_len; i++) {
			bool	valid = false;
			secp=  secpp[i];
			if ((secp->m_start >= RESET_ADDRESS)
				&&(secp->m_start+secp->m_len
						<= SPIFLASH+FLASHLEN))
				valid = true;
			if (!valid) {
				fprintf(stderr, "No such memory on board: 0x%08x - %08x\n",
					secp->m_start, secp->m_start+secp->m_len);
				if (!ignore_missing_memory)
					exit(EXIT_FAILURE);
			}
		}

		unsigned	startaddr = RESET_ADDRESS, codelen = 0;
		for(int i=0; secpp[i]->m_len; i++) {
			secp = secpp[i];

			unsigned start, idx, ln;

			start = secp->m_start;
			idx = 0;
			ln = secp->m_len;
			if (secp->m_start < SPIFLASH) {
				start = SPIFLASH;
				idx = SPIFLASH-secp->m_start;
				if (idx > secp->m_len)
					continue;
				ln = secp->m_len-idx;
			} if (start + ln > SPIFLASH+FLASHLEN) {
				if (start > SPIFLASH+FLASHLEN)
					continue;
				ln = SPIFLASH+FLASHLEN-start;
			}

			// We only ever write to the flash
			if (start < startaddr) {
				// Keep track of the first address in
				// flash, as well as the last address
				// that we will write
				codelen += (startaddr-secp->m_start);
				startaddr = secp->m_start;
			} if (start+ln > startaddr+codelen) {
				codelen = secp->m_start+secp->m_len-startaddr;
			} memcpy(&fbuf[start-SPIFLASH], &secp->m_data[idx], ln);
		}
		if ((flash)&&(!flash->write(startaddr, codelen, &fbuf[startaddr-SPIFLASH], true))) {
			fprintf(stderr, "ERR: Could not write program to flash\n");
			exit(EXIT_FAILURE);
		} else if (!flash)
			printf("flash->write(%08x, %d, ... );\n", startaddr,
				codelen);
		if (m_fpga) m_fpga->readio(R_VERSION); // Check for bus errors

		// Now ... how shall we start this CPU?
	} catch(BUSERR a) {
		fprintf(stderr, "S6-BUS error: %08x\n", a.addr);
		exit(-2);
	}

	if (flash) delete	flash;
	if (m_fpga) delete	m_fpga;

	return EXIT_SUCCESS;
}

