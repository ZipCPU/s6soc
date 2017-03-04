////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	zip_sim.cpp
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	This provides a simulation capability for the CMod S6 SoC.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#include "verilated.h"
#include "Vbusmaster.h"

#include "regdefs.h"
#include "testb.h"
// #include "twoc.h"
#include "qspiflashsim.h"
#include "uartsim.h"

typedef	uint32_t	BUSW;

class	GPIOSIM {
public:
	unsigned operator()(const unsigned o_gpio) { return 0; }
};

class	KEYPADSIM {
public:
	unsigned operator()(const unsigned o_kpd) { return 0; }
};

// Add a reset line, since Vbusmaster doesn't have one
class	Vbusmasterr : public Vbusmaster {
public:
	int	i_rst;
};

// No particular "parameters" need definition or redefinition here.
class	ZIPSIM_TB : public TESTB<Vbusmasterr> {
public:
	QSPIFLASHSIM	m_flash;
	UARTSIM		m_uart;
	GPIOSIM		m_gpio;
	KEYPADSIM	m_keypad;
	unsigned	m_last_led;
	time_t		m_start_time;
	FILE		*m_dbg;

	ZIPSIM_TB(void) : m_uart(0x2b6) {
		m_start_time = time(NULL);
		m_dbg = fopen("dbg.txt","w");
	}

	void	reset(void) {
		m_flash.debug(false);
	}

	void	tick(void) {
		if ((m_tickcount & ((1<<28)-1))==0) {
			double	ticks_per_second = m_tickcount;
			time_t	nsecs = (time(NULL)-m_start_time);
			if (nsecs > 0) {
				ticks_per_second /= (double)nsecs;
				printf(" ********   %.6f TICKS PER SECOND\n", 
					ticks_per_second);
			}
		}

		// Set up the bus before any clock tick

		// We've got the flash to deal with ...
		m_core->i_qspi_dat = m_flash(m_core->o_qspi_cs_n,
						m_core->o_qspi_sck,
						m_core->o_qspi_dat);

		// And the GPIO lines
		m_core->i_gpio = m_gpio(m_core->o_gpio);

		m_core->i_btn = 0; // 2'b0
		// o_led, o_pwm, o_pwm_aux

		// And the keypad
		m_core->i_kp_row = m_keypad(m_core->o_kp_col);

		// And the UART
		m_core->i_rx_stb  = m_uart.rx(m_core->i_rx_data);
		m_core->i_tx_busy = m_uart.tx(m_core->o_tx_stb, m_core->o_tx_data);

		TESTB<Vbusmasterr>::tick();

		if (m_core->o_led != m_last_led) {
			printf("LED: %08x\r", m_core->o_led); fflush(stdout);
			m_last_led = m_core->o_led;
		}

		
		if (m_dbg) fprintf(m_dbg, "%10ld - PC: %08x:%08x [%08x:%08x:%08x:%08x:%08x],%08x,%08x,%d,%08x,%08x (%x,%x/0x%08x)\n",
			m_tickcount,
			m_core->v__DOT__swic__DOT__thecpu__DOT__ipc,
			m_core->v__DOT__swic__DOT__thecpu__DOT__r_upc,
			m_core->v__DOT__swic__DOT__thecpu__DOT__regset[0],
			m_core->v__DOT__swic__DOT__thecpu__DOT__regset[1],
			m_core->v__DOT__swic__DOT__thecpu__DOT__regset[2],
			m_core->v__DOT__swic__DOT__thecpu__DOT__regset[3],
			m_core->v__DOT__swic__DOT__thecpu__DOT__regset[15],
			m_core->v__DOT__swic__DOT__thecpu__DOT__instruction_decoder__DOT__r_I,
			m_core->v__DOT__swic__DOT__thecpu__DOT__r_op_Bv,
			m_core->v__DOT__swic__DOT__thecpu__DOT__instruction_decoder__DOT__w_dcdR_pc,
			m_core->v__DOT__swic__DOT__thecpu__DOT__r_op_Av,
			m_core->v__DOT__swic__DOT__thecpu__DOT__wr_gpreg_vl,
			m_core->v__DOT__swic__DOT__thecpu__DOT__w_iflags,
			m_core->v__DOT__swic__DOT__thecpu__DOT__w_uflags,
			m_core->v__DOT__swic__DOT__thecpu__DOT__pf_pc
			);
		if ((!m_core->o_qspi_cs_n)&&(m_dbg))
			fprintf(m_dbg, "QSPI: [CS,SCK,DAT (MOD)] = %d,%d,%02x,%d -> %04x %7s, state= %x/(%d)\n",
				m_core->o_qspi_cs_n,
				m_core->o_qspi_sck,
				m_core->o_qspi_dat,
				m_core->o_qspi_mod,
				m_core->i_qspi_dat,
				(m_core->v__DOT__flashmem__DOT__quad_mode_enabled)?"(quad)":"",
				m_core->v__DOT__flashmem__DOT__state,
				m_core->v__DOT__flashmem__DOT__lldriver__DOT__state);

		if ((m_core->v__DOT__wb_cyc)&&(m_dbg))
		fprintf(m_dbg, "WB: %s/%s/%s[@0x%08x] %08x ->%s/%s %08x\n", 
			(m_core->v__DOT__wb_cyc)?"CYC":"   ",
			(m_core->v__DOT__wb_stb)?"STB":"   ",
			(m_core->v__DOT__wb_we )?"WE ":"   ",
			(m_core->v__DOT__w_zip_addr),
#define	wb_data	v__DOT__swic__DOT__thecpu__DOT__mem_data
			(m_core->wb_data),
			(m_core->v__DOT__wb_ack)?"ACK":"   ",
			(m_core->v__DOT__wb_stall)?"STL":"   ",
			(m_core->v__DOT__wb_idata)
			);
		/*
		if (m_dbg)
			fprintf(m_dbg, "PIC: %3s(%4x) %3s(%4x)%s\n",
				(m_core->v__DOT__pic__DOT__r_gie)?"GIE":"",
				(m_core->v__DOT__pic__DOT__r_int_enable),
				(m_core->v__DOT__pic__DOT__r_any)?"ANY":"",
				(m_core->v__DOT__pic__DOT__r_int_state),
				(m_core->v__DOT__pic__DOT__r_interrupt)?" ---> INT!":"");
		*/

		if ((m_core->v__DOT__swic__DOT__thecpu__DOT__pf_valid)&&(m_dbg))
			fprintf(m_dbg, "PC: %08x - %08x, uart=%d,%d, pic = %d,%04x,%0d,%04x\n",
				m_core->v__DOT__swic__DOT__thecpu__DOT__pf_instruction_pc,
				m_core->v__DOT__swic__DOT__thecpu__DOT__pf_instruction,
				m_core->i_rx_stb, m_core->i_tx_busy,
				m_core->v__DOT__pic__DOT__r_gie,
				m_core->v__DOT__pic__DOT__r_int_enable,
				m_core->v__DOT__pic__DOT__r_any,
				m_core->v__DOT__pic__DOT__r_int_state);
	}
};

ZIPSIM_TB	*tb;

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
	} flen /= sizeof(BUSW);
	return flen;
}

class	SECTION {
public:
	unsigned	m_start, m_len;
	BUSW	m_data[1];
};

SECTION	**singlesection(int nwords) {
	fprintf(stderr, "NWORDS = %d\n", nwords);
	size_t	sz = (2*(sizeof(SECTION)+sizeof(SECTION *))
		+(nwords-1)*(sizeof(BUSW)));
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

	if ((num_words=fgetwords(fp)) > FLASHWORDS) {
		fprintf(stderr, "File overruns flash memory\n");
		exit(-1);
	}
	secpp = singlesection(num_words);
	secp = secpp[0];
	secp->m_start = RESET_ADDRESS;
	secp->m_len = num_words;
	nr= fread(secp->m_data, sizeof(BUSW), num_words, fp);
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

		r[i]->m_start = phdr.p_vaddr;
		r[i]->m_len   = phdr.p_filesz/ sizeof(BUSW);

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


void	usage(void) {
	fprintf(stderr, "Usage: zip_sim flash_program\n");
}

int	main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	tb = new ZIPSIM_TB;
	const char	*codef = NULL;

	for(int argn=1; argn<argc; argn++) {
		if (argv[argn][0] == '-') {
			usage();
			exit(-1);
		} else
			codef = argv[argn];
	}

	if ((!codef)||(!codef[0]))
		fprintf(stderr, "No executable code filename found!\n");

	if (access(codef, R_OK)!=0)
		fprintf(stderr, "Cannot read code filename, %s\n", codef);

	if (iself(codef)) {
		SECTION	**secpp, *secp;
		BUSW	entry;
		elfread(codef, entry, secpp);

		for(int i=0; secpp[i]->m_len; i++) {
			secp = secpp[i];
			tb->m_flash.write(secp->m_start, secp->m_len, secp->m_data);
		}
	} else {
		tb->m_flash.load(RESET_ADDRESS, codef);
	}

	tb->reset();

	while(1)
		tb->tick();

	printf("SUCCESS!\n");
	exit(0);
}
