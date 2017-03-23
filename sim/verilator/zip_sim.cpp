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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#include "verilated.h"
#include "cpudefs.h"
#include "Vbusmaster.h"

#include "regdefs.h"
#include "testb.h"
// #include "twoc.h"
#include "qspiflashsim.h"
#include "uartsim.h"
#include "zipelf.h"
#include "byteswap.h"

typedef	uint32_t	BUSW;

class	GPIOSIM {
public:
	unsigned operator()(const unsigned o_gpio) { return 0; }
};

class	KEYPADSIM {
public:
	unsigned operator()(const unsigned o_kpd) { return 0; }
};

#define	LOWLOGIC_FLASH
#define	USE_LITE_UART

#define	tx_busy		v__DOT__tcvuart__DOT__r_busy
#define	rx_stb		v__DOT__rx_stb
#define	uart_setup	v__DOT__tcvuart__DOT__r_setup
#define	cpu_regset	v__DOT__swic__DOT__thecpu__DOT__regset
#define	cpu_gie		v__DOT__swic__DOT__thecpu__DOT__r_gie
#define	cpu_new_pc	v__DOT__swic__DOT__thecpu__DOT__new_pc
#define	cpu_ipc		v__DOT__swic__DOT__thecpu__DOT__ipc
#define	cpu_upc		v__DOT__swic__DOT__thecpu__DOT__r_upc
#define	cpu_op_Av	v__DOT__swic__DOT__thecpu__DOT__r_op_Av
#define	cpu_op_Bv	v__DOT__swic__DOT__thecpu__DOT__r_op_Bv
#define	cpu_iflags	v__DOT__swic__DOT__thecpu__DOT__w_iflags
#define	cpu_uflags	v__DOT__swic__DOT__thecpu__DOT__w_uflags
#define	cpu_pf_valid	v__DOT__swic__DOT__thecpu__DOT__pf_valid
#define	cpu_pf_pc	v__DOT__swic__DOT__thecpu__DOT__pf_pc
#ifdef	OPT_SINGLE_FETCH
#define	cpu_pf_instruction_pc	v__DOT__swic__DOT__thecpu__DOT__pf_addr
#else
#define	cpu_pf_instruction_pc	v__DOT__swic__DOT__thecpu__DOT__pf_instruction_pc
#endif
#define	cpu_pf_instruction	v__DOT__swic__DOT__thecpu__DOT__pf_instruction
#define	cpu_dcd_valid	v__DOT__swic__DOT__thecpu__DOT__instruction_decoder__DOT__r_valid
#define	cpu_dcd_iword	v__DOT__swic__DOT__thecpu__DOT__instruction_decoder__DOT__iword
#define	cpu_op_valid	v__DOT__swic__DOT__thecpu__DOT__op_valid
#define	cpu_op_sim	v__DOT__swic__DOT__thecpu__DOT__op_sim
#define	cpu_sim_immv	v__DOT__swic__DOT__thecpu__DOT__op_sim_immv
#define	cpu_dcd_ce	v__DOT__swic__DOT__thecpu__DOT__dcd_ce
#define	cpu_alu_ce	v__DOT__swic__DOT__thecpu__DOT__alu_ce
#define	cpu_wr_reg_ce	v__DOT__swic__DOT__thecpu__DOT__wr_reg_ce
#define	cpu_wr_reg_id	v__DOT__swic__DOT__thecpu__DOT__wr_reg_id
#define	cpu_wr_gpreg_vl	v__DOT__swic__DOT__thecpu__DOT__wr_gpreg_vl
#define	cpu_wr_flags_ce	v__DOT__swic__DOT__thecpu__DOT__wr_flags_ce
//
#define	pic_gie		v__DOT__pic__DOT__r_gie
#define	pic_int_enable	v__DOT__pic__DOT__r_int_enable
#define	pic_any		v__DOT__pic__DOT__r_any
#define	pic_int_state	v__DOT__pic__DOT__r_int_state
#define	wb_cyc		v__DOT__wb_cyc
#define	wb_stb		v__DOT__wb_stb
#define	wb_we		v__DOT__wb_we
#define	wb_data		v__DOT__swic__DOT__thecpu__DOT__mem_data
#define	wb_addr		v__DOT__w_zip_addr
//
#define	wb_ack		v__DOT__wb_ack
#define	wb_err		v__DOT__wb_err
#define	wb_stall	v__DOT__wb_stall
#define	wb_idata	v__DOT__wb_idata
//
#define	watchdog_int	v__DOT__watchdog_int

// No particular "parameters" need definition or redefinition here.
class	ZIPSIM_TB : public TESTB<Vbusmaster> {
public:
	QSPIFLASHSIM	m_flash;
	UARTSIM		m_uart;
	GPIOSIM		m_gpio;
	KEYPADSIM	m_keypad;
	unsigned	m_last_led;
	unsigned	m_last_gpio, m_last_pf_pc;
#ifdef	LOWLOGIC_FLASH
	unsigned	m_last_qspi_dat;
#endif
	time_t		m_start_time;
	FILE		*m_dbg;
	int		m_serial_port;

	ZIPSIM_TB(int serial_port, bool debug) : m_uart(serial_port),
				m_serial_port(serial_port) {
		m_start_time = time(NULL);
		if (debug)
			m_dbg = fopen("dbg.txt","w");
		else	m_dbg = NULL;

		m_last_led = m_last_gpio = m_last_pf_pc = -1;
#ifdef	LOWLOGIC_FLASH
		m_last_qspi_dat = 15;
#endif
	}

	void	reset(void) {
		m_flash.debug(false);
	}

	void	close(void) {
		closetrace();
	}

	void dump(FILE *fp, const uint32_t *regp) {
		uint32_t	uccv, iccv;
		fprintf(fp,"\nZIPM--DUMP: ");
		if (m_core->cpu_gie)
			fprintf(fp, "Interrupts-enabled\n");
		else
			fprintf(fp, "Supervisor mode\n");
		fprintf(fp, "\n");

		iccv = m_core->cpu_iflags;
		uccv = m_core->cpu_uflags;

		fprintf(fp, "sR0 : %08x ", regp[0]);
		fprintf(fp, "sR1 : %08x ", regp[1]);
		fprintf(fp, "sR2 : %08x ", regp[2]);
		fprintf(fp, "sR3 : %08x\n",regp[3]);
		fprintf(fp, "sR4 : %08x ", regp[4]);
		fprintf(fp, "sR5 : %08x ", regp[5]);
		fprintf(fp, "sR6 : %08x ", regp[6]);
		fprintf(fp, "sR7 : %08x\n",regp[7]);
		fprintf(fp, "sR8 : %08x ", regp[8]);
		fprintf(fp, "sR9 : %08x ", regp[9]);
		fprintf(fp, "sR10: %08x ", regp[10]);
		fprintf(fp, "sR11: %08x\n",regp[11]);
		fprintf(fp, "sR12: %08x ", regp[12]);
		fprintf(fp, "sSP : %08x ", regp[13]);
		fprintf(fp, "sCC : %08x ", iccv);
		fprintf(fp, "sPC : %08x\n",m_core->cpu_ipc);

		fprintf(fp, "\n");

		fprintf(fp, "uR0 : %08x ", regp[16]);
		fprintf(fp, "uR1 : %08x ", regp[17]);
		fprintf(fp, "uR2 : %08x ", regp[18]);
		fprintf(fp, "uR3 : %08x\n",regp[19]);
		fprintf(fp, "uR4 : %08x ", regp[20]);
		fprintf(fp, "uR5 : %08x ", regp[21]);
		fprintf(fp, "uR6 : %08x ", regp[22]);
		fprintf(fp, "uR7 : %08x\n",regp[23]);
		fprintf(fp, "uR8 : %08x ", regp[24]);
		fprintf(fp, "uR9 : %08x ", regp[25]);
		fprintf(fp, "uR10: %08x ", regp[26]);
		fprintf(fp, "uR11: %08x\n",regp[27]);
		fprintf(fp, "uR12: %08x ", regp[28]);
		fprintf(fp, "uSP : %08x ", regp[29]);
		fprintf(fp, "uCC : %08x ", uccv);
		fprintf(fp, "uPC : %08x\n",m_core->cpu_upc);
		fprintf(fp,"\n");
	}

	void	execsim(const uint32_t imm) {
		uint32_t	*regp = m_core->cpu_regset;
		int		rbase;
		rbase = (m_core->cpu_gie)?16:0;

		fflush(stdout);
		if ((imm & 0x03fffff)==0)
			return;
		// fprintf(stderr, "SIM-INSN(0x%08x)\n", imm);
		if ((imm & 0x0fffff)==0x00100) {
			// SIM Exit(0)
			close();
			exit(0);
		} else if ((imm & 0x0ffff0)==0x00310) {
			// SIM Exit(User-Reg)
			int	rcode;
			rcode = regp[(imm&0x0f)+16] & 0x0ff;
			close();
			exit(rcode);
		} else if ((imm & 0x0ffff0)==0x00300) {
			// SIM Exit(Reg)
			int	rcode;
			rcode = regp[(imm&0x0f)+rbase] & 0x0ff;
			close();
			exit(rcode);
		} else if ((imm & 0x0fff00)==0x00100) {
			// SIM Exit(Imm)
			int	rcode;
			rcode = imm & 0x0ff;
			close();
			exit(rcode);
		} else if ((imm & 0x0fffff)==0x002ff) {
			// Full/unconditional dump
			printf("SIM-DUMP\n");
			dump(stdout, regp);
			if ((m_dbg)&&(m_dbg != stdout)) {
				fprintf(m_dbg, "SIM-DUMP\n");
				dump(m_dbg, regp);
			}
		} else if ((imm & 0x0ffff0)==0x00200) {
			// Dump a register
			int rid = (imm&0x0f)+rbase;
			printf("%8ld @%08x R[%2d] = 0x%08x\n", m_tickcount,
				m_core->cpu_ipc, rid, regp[rid]);
			if ((m_dbg)&&(m_dbg != stdout))
				fprintf(m_dbg, "%8ld @%08x R[%2d] = 0x%08x\n",
					m_tickcount, m_core->cpu_ipc,
					rid, regp[rid]);
		} else if ((imm & 0x0ffff0)==0x00210) {
			// Dump a user register
			int rid = (imm&0x0f);
			printf("%8ld @%08x uR[%2d] = 0x%08x\n", m_tickcount,
				m_core->cpu_ipc, rid, regp[rid+16]);
			if ((m_dbg)&&(m_dbg != stdout))
				fprintf(m_dbg, "%8ld @%08x uR[%2d] = 0x%08x\n",
					m_tickcount, m_core->cpu_ipc,
					rid, regp[rid+16]);
		} else if ((imm & 0x0ffff0)==0x00230) {
			// SOUT[User Reg]
			int rid = (imm&0x0f)+16;
			printf("%c", regp[rid]&0x0ff);
			if ((m_dbg)&&(m_dbg != stdout))
				fprintf(m_dbg, "NOUT: %c\n", regp[rid]&0x0ff);
		} else if ((imm & 0x0fffe0)==0x00220) {
			// SOUT[User Reg]
			int rid = (imm&0x0f)+rbase;
			printf("%c", regp[rid]&0x0ff);
			if ((m_dbg)&&(m_dbg != stdout))
				fprintf(m_dbg, "NOUT: %c\n", regp[rid]&0x0ff);
		} else if ((imm & 0x0fff00)==0x00400) {
			// SOUT[Imm]
			printf("%c", imm&0x0ff);
			if ((m_dbg)&&(m_dbg != stdout))
				fprintf(m_dbg, "NOUT: %c\n", imm&0x0ff);
		} else { // if ((insn & 0x0f7c00000)==0x77800000)
			uint32_t	immv = imm & 0x03fffff;
			// Simm instruction that we dont recognize
			// if (imm)
			// printf("SIM 0x%08x\n", immv);
			printf("SIM 0x%08x (ipc = %08x, upc = %08x)\n", immv,
				m_core->cpu_ipc, m_core->cpu_upc);
			if ((m_dbg)&&(m_dbg != stdout))
				fprintf(m_dbg, "SIM 0x%08x (ipc = %08x, upc = %08x)\n",
					immv, m_core->cpu_ipc, m_core->cpu_upc);
		} fflush(stdout);
	}

	void	tick(void) {
		if ((m_tickcount & ((1<<28)-1))==0) {
			double	ticks_per_second = m_tickcount;
			time_t	nsecs = (time(NULL)-m_start_time);
			if ((nsecs > 0)&&(ticks_per_second>0)) {
				ticks_per_second /= (double)nsecs;
				printf(" ********   %.6f TICKS PER SECOND\n",
					ticks_per_second);
			}
		}

		// Set up the bus before any clock tick

		// We've got the flash to deal with ...
		int	iqspi;
#ifdef	LOWLOGIC_FLASH
		// The toplevel logic creates a one cycle/clock delay.  Emulate
		// it here.
		m_core->i_qspi_dat = m_last_qspi_dat;
		if ((m_core->o_qspi_sck == 3)||(m_core->o_qspi_sck == 0)) {
			iqspi = m_flash(m_core->o_qspi_cs_n,
						m_core->o_qspi_sck&1,
						m_core->o_qspi_dat);
		} else {
			iqspi = m_flash(m_core->o_qspi_cs_n,
						(m_core->o_qspi_sck&2)?1:0,
						m_core->o_qspi_dat);
			iqspi = m_flash(m_core->o_qspi_cs_n,
						(m_core->o_qspi_sck&1)?1:0,
						m_core->o_qspi_dat);
		}
#else
		// If this assertion fails, we probably meant to define
		// LOWLOGIC_FLASH, and failed to do so.
		assert((m_core->o_qspi_sck & ~1)==0);
		iqspi = m_flash(m_core->o_qspi_cs_n,
						m_core->o_qspi_sck,
						m_core->o_qspi_dat);
#endif
		if (m_core->o_qspi_mod&2) {
			// If we are driving the lines, drive the inputs
			// as well.
			if (m_core->o_qspi_mod&1)
				; // 2'b11	// Not driving any lines
			else	// 2'b10
				iqspi = m_core->o_qspi_dat;
		} else {
			// 2'b0x
			//
			// Turn on top two bits (unused in SPI mode)
			iqspi |= 0xc;
			// Replace bottom bit with o_qspi_dat[0]
			iqspi &= 0xe; // Turn off bottom bit
			iqspi |= m_core->o_qspi_dat&1;
		}
#ifndef	LOWLOGIC_FLASH
		m_core->i_qspi_dat = iqspi;
#else
		m_last_qspi_dat = iqspi;
#endif

		// And the GPIO lines
		m_core->i_gpio = m_gpio(m_core->o_gpio);

		m_core->i_btn = 0; // 2'b0
		// o_led, o_pwm, o_pwm_aux

		// And the keypad
		m_core->i_kp_row = m_keypad(m_core->o_kp_col);

		// And the UART
		m_core->i_uart_cts_n = 0;
#ifdef	USE_LITE_UART
		m_uart.setup(25);
#else
		m_uart.setup(m_core->uart_setup);
#endif
		m_core->i_uart  = m_uart(m_core->o_uart);

		TESTB<Vbusmaster>::tick();

		if ((m_core->o_led != m_last_led)||(m_core->o_gpio != m_last_gpio)) {

			char	ledstr[80];
			sprintf(ledstr, "LED: %x\tGPIO: %04x\tPF-PC = %08x",
				m_core->o_led, m_core->o_gpio,
				m_core->cpu_pf_pc);
			if (m_dbg) {
				printf("%s\n", ledstr);
				fprintf(m_dbg, "%s\n", ledstr);
			} else {
				printf("%s\r", ledstr);
				fflush(stdout);
			}
			m_last_led  = m_core->o_led;
			m_last_gpio = m_core->o_gpio;
			m_last_pf_pc = m_core->cpu_pf_pc;
		} else if ((!m_dbg)&&(m_serial_port != 0)
				&&(m_last_pf_pc != m_core->cpu_pf_pc)) {
			char	ledstr[80];
			sprintf(ledstr, "LED: %x\tGPIO: %04x\tPF-PC = %08x",
				m_core->o_led, m_core->o_gpio,
				m_core->cpu_pf_pc);
			printf("%s\r", ledstr);
			fflush(stdout);

			m_last_pf_pc = m_core->cpu_pf_pc;
		}

		if (m_core->watchdog_int) {
			printf("\nWATCHDOG-INT!!! CPU-sPC = %08x, TICKS = %08lx\n", m_core->cpu_ipc, m_tickcount);
		}

		if (m_dbg) {
			fprintf(m_dbg, "%10ld - ", m_tickcount);
			fprintf(m_dbg, "WB: %s%s%s/%s%s%s[@0x%08x] %08x->%08x",
				(m_core->wb_cyc)?"CYC":"   ",
				(m_core->wb_stb)?"/STB":"    ",
				(m_core->wb_we )?"W":"R",
				(m_core->wb_ack)?"ACK":"   ",
				(m_core->wb_stall)?"STL":"   ",
				(m_core->wb_err)?"ERR":"   ",
				(m_core->wb_addr<<2),
				(m_core->wb_data),
				(m_core->wb_idata)
				);

			fprintf(m_dbg, " CPU: %s%08x:%08x",
				(m_core->cpu_new_pc)?"NEW-":"    ",
				m_core->cpu_ipc,
				m_core->cpu_upc);
			fprintf(m_dbg, " PF:%08x->%08x@%08x%s",
				m_core->cpu_pf_pc,
				m_core->cpu_pf_instruction,
				(m_core->cpu_pf_instruction_pc*4),
				(m_core->cpu_pf_valid)?"V":" ");
			fprintf(m_dbg, " [I=%08x%s%s] OP[%08x,%08x] %s[%02x]%08x %s:%08x/%08x",
				m_core->cpu_dcd_iword,
				(m_core->cpu_dcd_valid)?"V":" ",
				(m_core->cpu_dcd_ce)?"CE":"  ",
				m_core->cpu_op_Av,
				m_core->cpu_op_Bv,
				(m_core->cpu_wr_reg_ce)?"WR":"  ",
				m_core->cpu_wr_reg_id,
				m_core->cpu_wr_gpreg_vl,
				(m_core->cpu_wr_flags_ce)?"WF":"  ",
				m_core->cpu_iflags,
				m_core->cpu_uflags);

			// QSPI flash
			fprintf(m_dbg, "QSPI: %s%s%02x|%d->%02x%6s, state= %x/(%d)",
				(m_core->o_qspi_cs_n)?"  ":"CS",
				(m_core->o_qspi_sck)?"CK":"  ",
				m_core->o_qspi_dat,
				m_core->o_qspi_mod,
				m_core->i_qspi_dat,
#ifdef	LOWLOGIC_FLASH
				"(quad)",0,0
#else
#define	flash_quad_mode_enabled v__DOT__flashmem__DOT__quad_mode_enabled
#define	flash_state		v__DOT__flashmem__DOT__state
#define	flash_llstate		v__DOT__flashmem__DOT__lldriver__DOT__state
				(m_core->flash_quad_mode_enabled)?"(quad)":"",
				m_core->flash_state
				m_core->v__DOT__flashmem__DOT__lldriver__DOT__state
#endif
			);

			fprintf(m_dbg, "\n");
		}

// SIM instruction(s)
		if ((m_core->cpu_op_sim)&&(m_core->cpu_op_valid)
			&&(m_core->cpu_alu_ce))
			execsim(m_core->cpu_sim_immv);
	}
};

void	usage(void) {
	fprintf(stderr, "Usage: zip_sim flash_program\n");
}

int	main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	ZIPSIM_TB	*tb;
	const char	*codef = NULL, *trace_file = NULL;
	bool	debug_flag = false;
	int	serial_port = -1;

	for(int argn=1; argn<argc; argn++) {
		if (argv[argn][0] == '-') for (int j=1;
					(j<512)&&(argv[argn][j]); j++) {
			switch(tolower(argv[argn][j])) {
			case 'c': break; // no comms to copy to stdout break;
			case 'd': debug_flag = true;
				if (trace_file == NULL)
					trace_file = "trace.vcd";
				break;
			case 'p': break; // S6 has no fpga command port
			case 's': serial_port=atoi(argv[++argn]); j=1000; break;
			case 't': trace_file = (argn+1<argc)?argv[++argn]:NULL;j=1000; break;
			case 'h': usage(); exit(EXIT_SUCCESS); break;
			default:
				fprintf(stderr, "ERR: Unexpected flag, -%c\n\n",
					argv[argn][j]);
				usage(); exit(EXIT_FAILURE);
			}
		} else if (iself(argv[argn])) {
			codef = argv[argn];
		} else {
			fprintf(stderr, "ERR: Unknown/unexpected argument: %s\n",
				argv[argn]);
			exit(EXIT_FAILURE);
		}
	}

	if ((!codef)||(!codef[0]))
		fprintf(stderr, "No executable code filename found!\n");

	if (serial_port < 0) {
		printf("Using the terminal as a SERIAL port\n");
		serial_port = 0;
	}
	tb = new ZIPSIM_TB(serial_port, debug_flag);

	if (access(codef, R_OK)!=0)
		fprintf(stderr, "Cannot read code filename, %s\n", codef);

	if (iself(codef)) {
		ELFSECTION	**secpp, *secp;
		BUSW		entry;
		elfread(codef, entry, secpp);

		assert(entry == RESET_ADDRESS);

		for(int i=0; secpp[i]->m_len; i++) {
			secp = secpp[i];
			tb->m_flash.write(secp->m_start, secp->m_len, (char *)&secp->m_data);
		}
	} else {
		fprintf(stderr, "%s is not a ZipCPU ELF executable\n", codef);
		exit(EXIT_FAILURE);
	}

	// if (debug_flag) { }
	if (trace_file)
		tb->opentrace(trace_file);

	tb->reset();

	while(1)
		tb->tick();

	printf("SUCCESS!\n");
	exit(0);
}
