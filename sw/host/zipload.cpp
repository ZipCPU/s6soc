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

bool	iself(const char *fname) {
	FILE	*fp;
	bool	ret = true;

	if ((!fname)||(!fname[0]))
		return false;

	fp = fopen(fname, "rb");

	if (!fp)	return false;
	if (0x7f != fgetc(fp))	ret = false;
	if ('E'  != fgetc(fp))	ret = false;
	if ('L'  != fgetc(fp))	ret = false;
	if ('F'  != fgetc(fp))	ret = false;
	fclose(fp);
	return 	ret;
}

long	fgetwords(FILE *fp) {
	// Return the number of words in the current file, and return the 
	// file as though it had never been adjusted
	long	fpos, flen;
	fpos = ftell(fp);
	if (0 != fseek(fp, 0l, SEEK_END)) {
		fprintf(stderr, "ERR: Could not determine file size\n");
		perror("O/S Err:");
		exit(-2);
	} flen = ftell(fp);
	if (0 != fseek(fp, fpos, SEEK_SET)) {
		fprintf(stderr, "ERR: Could not seek on file\n");
		perror("O/S Err:");
		exit(-2);
	} flen /= sizeof(FPGA::BUSW);
	return flen;
}

FPGA	*m_fpga;
class	SECTION {
public:
	unsigned	m_start, m_len;
	FPGA::BUSW	m_data[1];
};

SECTION	**singlesection(int nwords) {
	fprintf(stderr, "NWORDS = %d\n", nwords);
	size_t	sz = (2*(sizeof(SECTION)+sizeof(SECTION *))
		+(nwords-1)*(sizeof(FPGA::BUSW)));
	char	*d = (char *)malloc(sz);
	SECTION **r = (SECTION **)d;
	memset(r, 0, sz);
	r[0] = (SECTION *)(&d[2*sizeof(SECTION *)]);
	r[0]->m_len   = nwords;
	r[1] = (SECTION *)(&r[0]->m_data[r[0]->m_len]);
	r[0]->m_start = 0;
	r[1]->m_start = 0;
	r[1]->m_len   = 0;

	return r;
}

SECTION **rawsection(const char *fname) {
	SECTION		**secpp, *secp;
	unsigned	num_words;
	FILE		*fp;
	int		nr;

	fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "Could not open: %s\n", fname);
		exit(-1);
	}

	if ((num_words=fgetwords(fp)) > FLASHWORDS-(RESET_ADDRESS-SPIFLASH)) {
		fprintf(stderr, "File overruns flash memory\n");
		exit(-1);
	}
	secpp = singlesection(num_words);
	secp = secpp[0];
	secp->m_start = RAMBASE;
	secp->m_len = num_words;
	nr= fread(secp->m_data, sizeof(FPGA::BUSW), num_words, fp);
	if (nr != (int)num_words) {
		fprintf(stderr, "Could not read entire file\n");
		perror("O/S Err:");
		exit(-2);
	} assert(secpp[1]->m_len == 0);

	return secpp;
}

unsigned	byteswap(unsigned n) {
	unsigned	r;

	r = (n&0x0ff); n>>= 8;
	r = (r<<8) | (n&0x0ff); n>>= 8;
	r = (r<<8) | (n&0x0ff); n>>= 8;
	r = (r<<8) | (n&0x0ff); n>>= 8;

	return r;
}

// #define	CHEAP_AND_EASY
#ifdef	CHEAP_AND_EASY
#else
#include <libelf.h>
#include <gelf.h>

void	elfread(const char *fname, unsigned &entry, SECTION **&sections) {
	Elf	*e;
	int	fd, i;
	size_t	n;
	char	*id;
	Elf_Kind	ek;
	GElf_Ehdr	ehdr;
	GElf_Phdr	phdr;
	const	bool	dbg = false;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr, "ELF library initialization err, %s\n", elf_errmsg(-1));
		perror("O/S Err:");
		exit(EXIT_FAILURE);
	} if ((fd = open(fname, O_RDONLY, 0)) < 0) {
		fprintf(stderr, "Could not open %s\n", fname);
		perror("O/S Err:");
		exit(EXIT_FAILURE);
	} if ((e = elf_begin(fd, ELF_C_READ, NULL))==NULL) {
		fprintf(stderr, "Could not run elf_begin, %s\n", elf_errmsg(-1));
		exit(EXIT_FAILURE);
	}

	ek = elf_kind(e);
	if (ek == ELF_K_ELF) {
		; // This is the kind of file we should expect
	} else if (ek == ELF_K_AR) {
		fprintf(stderr, "Cannot run an archive!\n");
		exit(EXIT_FAILURE);
	} else if (ek == ELF_K_NONE) {
		;
	} else {
		fprintf(stderr, "Unexpected ELF file kind!\n");
		exit(EXIT_FAILURE);
	}

	if (gelf_getehdr(e, &ehdr) == NULL) {
		fprintf(stderr, "getehdr() failed: %s\n", elf_errmsg(-1));
		exit(EXIT_FAILURE);
	} if ((i=gelf_getclass(e)) == ELFCLASSNONE) {
		fprintf(stderr, "getclass() failed: %s\n", elf_errmsg(-1));
		exit(EXIT_FAILURE);
	} if ((id = elf_getident(e, NULL)) == NULL) {
		fprintf(stderr, "getident() failed: %s\n", elf_errmsg(-1));
		exit(EXIT_FAILURE);
	} if (i != ELFCLASS32) {
		fprintf(stderr, "This is a 64-bit ELF file, ZipCPU ELF files are all 32-bit\n");
		exit(EXIT_FAILURE);
	}

	if (dbg) {
	printf("    %-20s 0x%jx\n", "e_type", (uintmax_t)ehdr.e_type);
	printf("    %-20s 0x%jx\n", "e_machine", (uintmax_t)ehdr.e_machine);
	printf("    %-20s 0x%jx\n", "e_version", (uintmax_t)ehdr.e_version);
	printf("    %-20s 0x%jx\n", "e_entry", (uintmax_t)ehdr.e_entry);
	printf("    %-20s 0x%jx\n", "e_phoff", (uintmax_t)ehdr.e_phoff);
	printf("    %-20s 0x%jx\n", "e_shoff", (uintmax_t)ehdr.e_shoff);
	printf("    %-20s 0x%jx\n", "e_flags", (uintmax_t)ehdr.e_flags);
	printf("    %-20s 0x%jx\n", "e_ehsize", (uintmax_t)ehdr.e_ehsize);
	printf("    %-20s 0x%jx\n", "e_phentsize", (uintmax_t)ehdr.e_phentsize);
	printf("    %-20s 0x%jx\n", "e_shentsize", (uintmax_t)ehdr.e_shentsize);
	printf("\n");
	}


	// Check whether or not this is an ELF file for the ZipCPU ...
	if (ehdr.e_machine != 0x0dadd) {
		fprintf(stderr, "This is not a ZipCPU ELF file\n");
		exit(EXIT_FAILURE);
	}

	// Get our entry address
	entry = ehdr.e_entry;


	// Now, let's go look at the program header
	if (elf_getphdrnum(e, &n) != 0) {
		fprintf(stderr, "elf_getphdrnum() failed: %s\n", elf_errmsg(-1));
		exit(EXIT_FAILURE);
	}

	unsigned total_octets = 0, current_offset=0, current_section=0;
	for(i=0; i<(int)n; i++) {
		total_octets += sizeof(SECTION *)+sizeof(SECTION);

		if (gelf_getphdr(e, i, &phdr) != &phdr) {
			fprintf(stderr, "getphdr() failed: %s\n", elf_errmsg(-1));
			exit(EXIT_FAILURE);
		}

		if (dbg) {
		printf("    %-20s 0x%x\n", "p_type",   phdr.p_type);
		printf("    %-20s 0x%jx\n", "p_offset", phdr.p_offset);
		printf("    %-20s 0x%jx\n", "p_vaddr",  phdr.p_vaddr);
		printf("    %-20s 0x%jx\n", "p_paddr",  phdr.p_paddr);
		printf("    %-20s 0x%jx\n", "p_filesz", phdr.p_filesz);
		printf("    %-20s 0x%jx\n", "p_memsz",  phdr.p_memsz);
		printf("    %-20s 0x%x [", "p_flags",  phdr.p_flags);

		if (phdr.p_flags & PF_X)	printf(" Execute");
		if (phdr.p_flags & PF_R)	printf(" Read");
		if (phdr.p_flags & PF_W)	printf(" Write");
		printf("]\n");
		printf("    %-20s 0x%jx\n", "p_align", phdr.p_align);
		}

		total_octets += phdr.p_memsz;
	}

	char	*d = (char *)malloc(total_octets + sizeof(SECTION)+sizeof(SECTION *));
	memset(d, 0, total_octets);

	SECTION **r = sections = (SECTION **)d;
	current_offset = (n+1)*sizeof(SECTION *);
	current_section = 0;

	for(i=0; i<(int)n; i++) {
		r[i] = (SECTION *)(&d[current_offset]);

		if (gelf_getphdr(e, i, &phdr) != &phdr) {
			fprintf(stderr, "getphdr() failed: %s\n", elf_errmsg(-1));
			exit(EXIT_FAILURE);
		}

		if (dbg) {
		printf("    %-20s 0x%jx\n", "p_offset", phdr.p_offset);
		printf("    %-20s 0x%jx\n", "p_vaddr",  phdr.p_vaddr);
		printf("    %-20s 0x%jx\n", "p_paddr",  phdr.p_paddr);
		printf("    %-20s 0x%jx\n", "p_filesz", phdr.p_filesz);
		printf("    %-20s 0x%jx\n", "p_memsz",  phdr.p_memsz);
		printf("    %-20s 0x%x [", "p_flags",  phdr.p_flags);

		if (phdr.p_flags & PF_X)	printf(" Execute");
		if (phdr.p_flags & PF_R)	printf(" Read");
		if (phdr.p_flags & PF_W)	printf(" Write");
		printf("]\n");

		printf("    %-20s 0x%jx\n", "p_align", phdr.p_align);
		}

		current_section++;

		r[i]->m_start = phdr.p_paddr;
		r[i]->m_len   = phdr.p_filesz/ sizeof(FPGA::BUSW);

		current_offset += phdr.p_memsz + sizeof(SECTION);

		// Now, let's read in our section ...
		if (lseek(fd, phdr.p_offset, SEEK_SET) < 0) {
			fprintf(stderr, "Could not seek to file position %08lx\n", phdr.p_offset);
			perror("O/S Err:");
			exit(EXIT_FAILURE);
		} if (phdr.p_filesz > phdr.p_memsz)
			phdr.p_filesz = 0;
		if (read(fd, r[i]->m_data, phdr.p_filesz) != (int)phdr.p_filesz) {
			fprintf(stderr, "Didnt read entire section\n");
			perror("O/S Err:");
			exit(EXIT_FAILURE);
		}

		// Next, we need to byte swap it from big to little endian
		for(unsigned j=0; j<r[i]->m_len; j++)
			r[i]->m_data[j] = byteswap(r[i]->m_data[j]);

		if (dbg) for(unsigned j=0; j<r[i]->m_len; j++)
			fprintf(stderr, "ADR[%04x] = %08x\n", r[i]->m_start+j,
			r[i]->m_data[j]);
	}

	r[i] = (SECTION *)(&d[current_offset]);
	r[current_section]->m_start = 0;
	r[current_section]->m_len   = 0;

	elf_end(e);
	close(fd);
}
#endif

void	usage(void) {
	printf("USAGE: zipload [-h] [<bit-file> [<alt-bit-file>]] <zip-program-file>\n");
	printf("\n"
"\t-h\tDisplay this usage statement\n");
}

int main(int argc, char **argv) {
	int		skp=0;
	bool		permit_raw_files = false, debug_only = false;
	unsigned	entry = RAMBASE;
	FLASHDRVR	*flash = NULL;
	const char	*bitfile = NULL, *altbitfile = NULL;

	if (argc < 2) {
		usage();
		exit(EXIT_SUCCESS);
	}

	skp=1;
	for(int argn=0; argn<argc-skp; argn++) {
		if (argv[argn+skp][0] == '-') {
			switch(argv[argn+skp][1]) {
			case 'd':
				debug_only = true;
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;
			case 'r':
				permit_raw_files = true;
				break;
			default:
				fprintf(stderr, "Unknown option, -%c\n\n",
					argv[argn+skp][0]);
				usage();
				exit(EXIT_FAILURE);
				break;
			} skp++; argn--;
		} else { // Check for bit files
			int sl = strlen(argv[argn+skp]);
			if ((sl>4)&&(strcmp(&argv[argn+skp][sl-4],".bit")==0)) {
				if (bitfile == NULL)
					bitfile = argv[argn+skp];
				else if (altbitfile == NULL)
					altbitfile = argv[argn+skp];
				else {
					fprintf(stderr, "Err: Too many bit files listed\n");
					exit(EXIT_FAILURE);
				} skp++; argn--;
			} else
				argv[argn] = argv[argn+skp];
		}
	} argc -= skp;


	if ((bitfile)&&(access(bitfile,R_OK)!=0)) {
		fprintf(stderr, "Cannot open bitfile, %s\n", bitfile);
		exit(EXIT_FAILURE);
	} if ((altbitfile)&&(access(altbitfile,R_OK)!=0)) {
		fprintf(stderr, "Cannot open alternate bitfile, %s\n",
			altbitfile);
		exit(EXIT_FAILURE);
	} if(((!bitfile)&&(argc<=0)) || ((argc>0)&&(access(argv[0],R_OK)!=0))) {
		// If there's no code file, or the code file cannot be opened
		if (argc>0)
			fprintf(stderr, "Cannot open executable, %s\n", argv[0]);
		else
			usage();
		exit(EXIT_FAILURE);
	}

	const char *codef = (argc>0)?argv[0]:NULL;
	DEVBUS::BUSW	*fbuf = new DEVBUS::BUSW[FLASHWORDS];

	// Set the flash buffer to all ones
	memset(fbuf, -1, FLASHWORDS*sizeof(fbuf[0]));

	if (debug_only) {
		m_fpga = NULL;
	} else {
        	char    szSel[64];
        	strcpy(szSel, "SN:210282768825");
		m_fpga = new FPGA(new DEPPI(szSel));
	}

	try {
		unsigned v  = m_fpga->readio(R_VERSION);
		if (v < 0x20160000) {
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
		int	len;
		FILE	*fp = fopen(bitfile, "rb");

		fseek(fp, 0x5dl, SEEK_SET);
		len = fread(&fbuf[CONFIG_ADDRESS-SPIFLASH],
				sizeof(fbuf[0]),
				FLASHWORDS-(CONFIG_ADDRESS-SPIFLASH), fp);
		assert(len + CONFIG_ADDRESS < ALTCONFIG_ADDRESS);
		fclose(fp);

		for(int i=0; i<4; i++) {
			// printf("0x%08x\n", fbuf[i]);
			assert(fbuf[i] == 0x0ffffffff);
		} // printf("0x%08x\n", fbuf[4]);
		assert(fbuf[4] == 0x0665599aa);

		printf("Loading: %s\n", bitfile);
		if ((flash)&&(!flash->write(CONFIG_ADDRESS, len, &fbuf[CONFIG_ADDRESS-SPIFLASH], true))) {
			fprintf(stderr, "Could not write primary bitfile\n");
			exit(EXIT_FAILURE);
		}
	} if (altbitfile) {
		int	len;
		FILE	*fp = fopen(altbitfile, "rb");

		// The alternate configuration follows the first configuration
		len = fread(&fbuf[ALTCONFIG_ADDRESS-SPIFLASH],
				sizeof(fbuf[0]),
				FLASHWORDS-(ALTCONFIG_ADDRESS-SPIFLASH), fp);
		assert(len + ALTCONFIG_ADDRESS < RESET_ADDRESS);
		fclose(fp);
		printf("Loading: %s\n", altbitfile);

		if ((flash)&&(!flash->write(ALTCONFIG_ADDRESS, len, &fbuf[ALTCONFIG_ADDRESS-SPIFLASH], true))) {
			fprintf(stderr, "Could not write alternate bitfile\n");
			exit(EXIT_FAILURE);
		}
	}

	if (codef) try {
		SECTION	**secpp = NULL, *secp;

		if(iself(codef)) {
			// zip-readelf will help with both of these ...
			elfread(codef, entry, secpp);
			assert(entry == RESET_ADDRESS);
		} else if (permit_raw_files) {
			secpp = rawsection(codef);
			entry = RESET_ADDRESS;
		} else {
			fprintf(stderr, "ERR: %s is not in ELF format\n", codef);
			exit(EXIT_FAILURE);
		}

		printf("Loading: %s\n", codef);
		// assert(secpp[1]->m_len = 0);
		for(int i=0; secpp[i]->m_len; i++) {
			bool	valid = false;
			secp=  secpp[i];
			if ((secp->m_start >= RESET_ADDRESS)
				&&(secp->m_start+secp->m_len
						<= SPIFLASH+FLASHWORDS))
				valid = true;
			if ((secp->m_start >= RAMBASE)
				&&(secp->m_start+secp->m_len
						<= RAMBASE+MEMWORDS))
				valid = true;
			if (!valid) {
				fprintf(stderr, "No such memory on board: 0x%08x - %08x\n",
					secp->m_start, secp->m_start+secp->m_len);
				exit(EXIT_FAILURE);
			}
		}

		unsigned	startaddr = RESET_ADDRESS, codelen = 0;
		for(int i=0; secpp[i]->m_len; i++) {
			secp = secpp[i];
			if ((secp->m_start >= RAMBASE)
				&&(secp->m_start+secp->m_len
						<= RAMBASE+MEMWORDS)) {
				for(int i=0; (unsigned)i<secp->m_len; i++) {
					if (secp->m_data[i] != 0) {
						fprintf(stderr, "ERR: Cannot set RAM upon bootup!\n");
						fprintf(stderr, "(The bootloaders just not that smart ... yet)\n");
						fprintf(stderr, "Attempting to set %08x - %08x\n", secp->m_start, secp->m_start+secp->m_len-1);
						fprintf(stderr, "%08x cannot be set to %08x\n", secp->m_start+i, secp->m_data[i]);
						exit(EXIT_FAILURE);
					}
				}
			} else {
				if (secp->m_start < startaddr) {
					codelen += (startaddr-secp->m_start);
					startaddr = secp->m_start;
				} if (secp->m_start+secp->m_len > startaddr+codelen) {
					codelen = secp->m_start+secp->m_len-startaddr;
				} memcpy(&fbuf[secp->m_start-SPIFLASH],
					secp->m_data, 
					secp->m_len*sizeof(FPGA::BUSW));
			}
		}
		if ((flash)&&(!flash->write(startaddr, codelen, &fbuf[startaddr-SPIFLASH], true))) {
			fprintf(stderr, "ERR: Could not write program to flash\n");
			exit(EXIT_FAILURE);
		} else if (!flash)
			printf("flash->write(%08x, %d, ... );\n", startaddr,
				codelen);
		if (m_fpga) m_fpga->readio(R_VERSION); // Check for bus errors

		// Now ... how shall we start this CPU?
		printf("The CPU should be fully loaded, you may now start\n");
		printf("it.  To start the CPU, either toggle power or type\n");
		printf("%% wbregs fpgagen1 0 \n");
		printf("%% wbregs fpgagen2 0x0300 \n");
		printf("%% wbregs fpgacmd  14 \n");
	} catch(BUSERR a) {
		fprintf(stderr, "XULA-BUS error: %08x\n", a.addr);
		exit(-2);
	}

	if (m_fpga) delete	m_fpga;

	return EXIT_SUCCESS;
}

