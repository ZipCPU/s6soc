////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	busmaster.v
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	
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
//
`include "builddate.v"
//
`define	INCLUDE_ZIPPY
`define	IMPLEMENT_ONCHIP_RAM
// `ifndef	VERILATOR
// `define	FANCY_ICAP_ACCESS
// `endif
`define	FLASH_ACCESS
`define	DBG_SCOPE	// About 204 LUTs, at 2^6 addresses
// `define	COMPRESSED_SCOPE
`define	INCLUDE_SECOND_TIMER
`define	SECOND_TIMER_IS_WATCHDOG
// `define	INCLUDE_RTC	// About 90 LUTs
// `define	FULL_BUSERR_CALCULATION
`define	INCLUDE_CPU_RESET_LOGIC
module	busmaster(i_clk, i_rst,
		i_rx_stb, i_rx_data, o_tx_stb, o_tx_data, i_tx_busy,
			o_uart_cts,
		// The SPI Flash lines
		o_qspi_cs_n, o_qspi_sck, o_qspi_dat, i_qspi_dat, o_qspi_mod,
		// The board I/O
		i_btn, o_led, o_pwm, o_pwm_aux,
		// Keypad connections
		i_kp_row, o_kp_col,
		// UART control
		o_uart_setup,
		// GPIO lines
		i_gpio, o_gpio);
	parameter	BUS_ADDRESS_WIDTH=23,
			ZIP_ADDRESS_WIDTH=BUS_ADDRESS_WIDTH,
			CMOD_ZIPCPU_RESET_ADDRESS=23'h480000;
	localparam	ZA=ZIP_ADDRESS_WIDTH,
			BAW=BUS_ADDRESS_WIDTH; // 24bits->2,258,23b->2181
	input			i_clk, i_rst;
	input			i_rx_stb;
	input		[7:0]	i_rx_data;
	output	reg		o_tx_stb;
	output	reg	[7:0]	o_tx_data;
	input			i_tx_busy;
	output	wire		o_uart_cts;
	// SPI flash control
	output	wire		o_qspi_cs_n, o_qspi_sck;
	output	wire	[3:0]	o_qspi_dat;
	input		[3:0]	i_qspi_dat;
	output	wire	[1:0]	o_qspi_mod;
	// Board I/O
	input		[1:0]	i_btn;
	output	wire	[3:0]	o_led;
	output	wire		o_pwm;
	output	wire	[1:0]	o_pwm_aux;
	// Keypad
	input		[3:0]	i_kp_row;
	output	wire	[3:0]	o_kp_col;
	// UART control
	output	wire	[29:0]	o_uart_setup;
	// GPIO liines
	input		[15:0]	i_gpio;
	output	wire	[15:0]	o_gpio;


	//
	//
	// Master wishbone wires
	//
	//
	wire		wb_cyc, wb_stb, wb_we, wb_stall, wb_ack, wb_err;
	wire	[31:0]	wb_data, wb_idata;
	wire	[3:0]	wb_sel;
	wire	[(BAW-1):0]	wb_addr;
	wire	[5:0]		io_addr;
	assign	io_addr = {
			wb_addr[22],	// Flash
			wb_addr[13],	// RAM
			wb_addr[11],	// RTC
			wb_addr[10],	// CFG
			wb_addr[ 9],	// SCOPE
			wb_addr[ 8] };	// I/O

	// Wires going to devices
	// And then headed back home
	wire	w_interrupt;
	// Oh, and the debug control for the ZIP CPU
	wire		zip_dbg_ack, zip_dbg_stall;
	wire	[31:0]	zip_dbg_data;


	//
	//
	// The BUS master (source): The ZipCPU
	//
	//
	wire		zip_cyc, zip_stb, zip_we, zip_cpu_int;
	wire	[(ZA-1):0]	w_zip_addr;
	wire	[(BAW-1):0]	zip_addr;
	wire	[31:0]		zip_data, zip_scope_data;
	// and then coming from devices
	wire		zip_ack, zip_stall, zip_err;
	wire	dwb_we, dwb_stb, dwb_cyc, dwb_ack, dwb_stall, dwb_err;
	wire	[(BAW-1):0]	dwb_addr;
	wire	[31:0]		dwb_odata;

	// wire	[31:0]	zip_debug;
//
// We'll define our RESET_ADDRESS to be halfway through our flash memory.
//	`define	CMOD_ZIPCPU_RESET_ADDRESS	23'h600000
//
// Ahm, No.  We can actually do much better than that.  Our toplevel *.bit file
// only takes up only 335kB.  Let's give it some room to grow to 1024 kB.  Then
// 23 can start our ROM at 23'h400100
//
// Not so fast.  In hindsight, we really want to be  able to adjust the load and
// the program separately.  So, instead, let's place our RESET address at the
// second flash erase block.  That way, we can change our program code found
// in the flash without needing to change our FPGA load and vice versa.
//
// 23'h404000
	wire	cpu_reset, tmrb_int;
`ifdef	INCLUDE_CPU_RESET_LOGIC
	reg	btn_reset, x_button, r_button;
	initial	btn_reset = 1'b0;
	initial	x_button = 1'b0;
	initial	r_button = 1'b0;
	always @(posedge i_clk)
	begin
		x_button <= i_btn[1];
		r_button <= x_button;
`ifdef	SECOND_TIMER_IS_WATCHDOG
		btn_reset <= ((r_button)&&(zip_cpu_int))||(tmrb_int);
`else
		btn_reset <= ((r_button)&&(zip_cpu_int));
`endif
	end
	assign	cpu_reset = btn_reset;
`else
	assign	cpu_reset = 1'b0;
`endif

	zipbones #(CMOD_ZIPCPU_RESET_ADDRESS,ZA,6)
		thecpu(i_clk, btn_reset, // 1'b0,
			// Zippys wishbone interface
			wb_cyc, wb_stb, wb_we, w_zip_addr, wb_data, wb_sel,
				wb_ack, wb_stall, wb_idata, wb_err,
			w_interrupt, zip_cpu_int,
			// Debug wishbone interface -- not really used
			1'b0, 1'b0,1'b0, 1'b0, 32'h00,
				zip_dbg_ack, zip_dbg_stall, zip_dbg_data,
			zip_scope_data);
	generate
	if (ZA < BAW)
		assign	wb_addr = { {(BAW-ZA){1'b0}}, w_zip_addr };
	else
		assign	wb_addr = w_zip_addr;
	endgenerate

	wire	io_sel, flash_sel, flctl_sel, scop_sel, cfg_sel, mem_sel,
			rtc_sel, none_sel, many_sel;
	wire	flash_ack, scop_ack, cfg_ack, mem_ack, many_ack;
	wire	rtc_ack, rtc_stall;
`ifdef	INCLUDE_RTC
	assign	rtc_stall = 1'b0;
`endif
	wire	io_stall, flash_stall, scop_stall, cfg_stall, mem_stall;
	reg	io_ack;

	wire	[31:0]	flash_data, scop_data, cfg_data, mem_data, pwm_data,
			spio_data, gpio_data, uart_data;
	reg	[31:0]	io_data;
	reg	[(BAW-1):0]	bus_err_addr;

	assign	wb_ack = (wb_cyc)&&((io_ack)||(scop_ack)||(cfg_ack)
`ifdef	INCLUDE_RTC
				||(rtc_ack)
`endif
				||(mem_ack)||(flash_ack)||((none_sel)&&(1'b1)));
	assign	wb_stall = ((io_sel)&&(io_stall))
			||((scop_sel)&&(scop_stall))
			||((cfg_sel)&&(cfg_stall))
			||((mem_sel)&&(mem_stall))
`ifdef	INCLUDE_RTC
			||((rtc_sel)&&(rtc_stall))
`endif
			||((flash_sel||flctl_sel)&&(flash_stall));
			// (none_sel)&&(1'b0)

	/*
	assign	wb_idata = (io_ack)?io_data
			: ((scop_ack)?scop_data
			: ((cfg_ack)?cfg_data
			: ((mem_ack)?mem_data
			: ((flash_ack)?flash_data
			: 32'h00))));
	*/
	assign	wb_idata =  (io_ack|scop_ack)?((io_ack )? io_data  : scop_data)
			: ((mem_ack|rtc_ack)?((mem_ack)?mem_data:rtc_data)
			: ((cfg_ack) ? cfg_data : flash_data));//if (flash_ack)
	assign	wb_err = ((wb_cyc)&&(wb_stb)&&(none_sel || many_sel)) || many_ack;

	// Addresses ...
	//	0000 xxxx	configuration/control registers
	//	1 xxxx xxxx xxxx xxxx xxxx	Up-sampler taps
	assign	io_sel   =((wb_cyc)&&(io_addr[5:0]==6'h1));
	assign	scop_sel =((wb_cyc)&&(io_addr[5:1]==5'h1));
	assign	flctl_sel= 1'b0; // ((wb_cyc)&&(io_addr[5:1]==5'h1));
	assign	cfg_sel  =((wb_cyc)&&(io_addr[5:2]==4'h1));
	// zip_sel is not on the bus at this point
`ifdef	INCLUDE_RTC
	assign	rtc_sel  =((wb_cyc)&&(io_addr[5:3]==3'h1));
`endif
	assign	mem_sel  =((wb_cyc)&&(io_addr[5:4]==2'h1));
	assign	flash_sel=((wb_cyc)&&(io_addr[5]));

`ifdef	FULL_BUSERR_CALCULATION
	assign	none_sel =((wb_cyc)&&(wb_stb)&&
			((io_addr==6'h0)
			||((~io_addr[5])&&(|wb_addr[22:14]))
			||((io_addr[5:4]==2'b00)&&(|wb_addr[12])))
			);
	assign	many_sel =((wb_cyc)&&(wb_stb)&&(
			 {3'h0, io_sel}
			+{3'h0, flctl_sel}
			+{3'h0, scop_sel}
			+{3'h0, cfg_sel}
			+{3'h0, rtc_sel}
			+{3'h0, mem_sel}
			+{3'h0, flash_sel} > 1));

	assign	many_ack =((wb_cyc)&&(
			 {3'h0, io_ack}
			+{3'h0, scop_ack}
			+{3'h0, cfg_ack}
`ifdef	INCLUDE_RTC
			+{3'h0, rtc_ack}
`endif
			+{3'h0, mem_ack}
			+{3'h0, flash_ack} > 1));
`else
	assign	many_ack = 1'b0;
	assign	many_sel = 1'b0;
	assign	none_sel =((wb_cyc)&&(wb_stb)&&(
				(io_addr[5:4]==2'h0)
				&&(~io_addr[0])
`ifdef	INCLUDE_RTC
				&&(~io_addr[3])
`endif
`ifdef	FANCY_ICAP_ACCESS
				&&(~io_addr[2])
`endif
`ifdef	DBG_SCOPE
				&&(~io_addr[1])
`endif
				));
`endif
	wire		flash_interrupt, scop_interrupt, tmra_int,
			rtc_interrupt, gpio_int, pwm_int, keypad_int,button_int;


	//
	//
	//
	reg		rx_rdy;
	wire	[10:0]	int_vector;
	assign	int_vector = { 
					gpio_int, pwm_int, keypad_int,
				(~o_tx_stb), rx_rdy,
`ifdef	SECOND_TIMER_IS_WATCHDOG
				1'b0,
`else
				tmrb_int,
`endif
				tmra_int,
				rtc_interrupt, scop_interrupt,
				wb_err, button_int };

	wire	[31:0]	pic_data;
	icontrol #(11)	pic(i_clk, 1'b0, (wb_stb)&&(io_sel)
					&&(wb_addr[3:0]==4'h0)&&(wb_we),
			wb_data, pic_data, int_vector, w_interrupt);

	initial	bus_err_addr = 0; // `DATESTAMP;
	always @(posedge i_clk)
		if (wb_err)
			bus_err_addr <= wb_addr;

	wire	[31:0]	timer_a, timer_b;
	wire		zta_ack, zta_stall, ztb_ack, ztb_stall;
	ziptimer	#(32,31,1)
		zipt_a(i_clk, 1'b0, 1'b1, wb_cyc,
`ifdef	INCLUDE_SECOND_TIMER
				(wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h2),
`else
				(wb_stb)&&(io_sel)&&(wb_addr[3:1]==3'h1),
`endif
				wb_we, wb_data, zta_ack, zta_stall, timer_a,
				tmra_int);
`ifdef	INCLUDE_SECOND_TIMER
`ifdef	SECOND_TIMER_IS_WATCHDOG
	ziptimer	#(32,31,0)
		zipt_b(i_clk, cpu_reset, 1'b1, wb_cyc,
				(wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h3),
				wb_we, wb_data, ztb_ack, ztb_stall, timer_b,
				tmrb_int);
`else
	ziptimer	#(32,31,1)
		zipt_b(i_clk, cpu_reset, 1'b1, wb_cyc,
				(wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h3),
				wb_we, wb_data, ztb_ack, ztb_stall, timer_b,
				tmrb_int);
`endif
`else
	// assign	timer_b = 32'h000;
	assign	timer_b = timer_a;
	assign	tmrb_int = 1'b0;
`endif

	wire	[31:0]	rtc_data;
`ifdef	INCLUDE_RTC
	wire	rtcd_ack, rtcd_stall, ppd;
	// rtcdate	thedate(i_clk, ppd, wb_cyc, (wb_stb)&&(io_sel), wb_we,
			// wb_data, rtcd_ack, rtcd_stall, date_data);
	reg	r_rtc_ack;
	initial	r_rtc_ack = 1'b0;
	always @(posedge i_clk)
		r_rtc_ack <= ((wb_stb)&&(rtc_sel));
	assign	rtc_ack = r_rtc_ack;

	rtclight
		#(23'h35afe5,23,0,0) 	// 80 MHz clock
		thetime(i_clk, wb_cyc,
			((wb_stb)&&(rtc_sel)), wb_we,
			{ 1'b0, wb_addr[1:0] }, wb_data, rtc_data,
			rtc_interrupt, ppd);
`else
	assign	rtc_interrupt = 1'b0;
	assign	rtc_data = 32'h00;
	assign	rtc_ack  = 1'b0;
`endif

	always @(posedge i_clk)
		case(wb_addr[3:0])
			4'h0: io_data <= pic_data;
			4'h1: io_data <= { {(32-BAW){1'b0}}, bus_err_addr };
			4'h2: io_data <= timer_a;
			4'h3: io_data <= timer_b;
			4'h4: io_data <= pwm_data;
			4'h5: io_data <= spio_data;
			4'h6: io_data <= gpio_data;
			4'h7: io_data <= uart_data;
			default: io_data <= `DATESTAMP;
			// 4'h8: io_data <= `DATESTAMP;
		endcase
	always @(posedge i_clk)
		io_ack <= (wb_cyc)&&(wb_stb)&&(io_sel);
	assign	io_stall = 1'b0;

	wire	pwm_ack, pwm_stall;
	wbpwmaudio	#(14'd10000,2,0,14)
		theaudio(i_clk, wb_cyc,
				((wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h4)),
					wb_we, 1'b0, wb_data,
				pwm_ack, pwm_stall, pwm_data, o_pwm,
					o_pwm_aux, //={pwm_shutdown_n,pwm_gain}
					pwm_int);

	//
	// Special Purpose I/O: Keypad, button, LED status and control
	//
	wire	[3:0]	w_led;
	spio	thespio(i_clk, wb_cyc,(wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h5),wb_we,
			wb_data, spio_data, o_kp_col, i_kp_row, i_btn, w_led,
			keypad_int, button_int);
	assign	o_led = { w_led[3]|w_interrupt,w_led[2]|zip_cpu_int,w_led[1:0] };

	//
	// General purpose (sort of) I/O:  (Bottom two bits robbed in each
	// direction for an I2C link at the toplevel.v design)
	//
	wbgpio	#(16,16,16'hffff) thegpio(i_clk, wb_cyc,
			(wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h6), wb_we,
			wb_data, gpio_data, i_gpio, o_gpio, gpio_int);

	//
	//
	//	Rudimentary serial port control
	//
	reg	[7:0]	r_rx_data;
	// Baud rate is set by clock rate / baud rate.
	// Thus, 80MHz / 115200MBau
	//	= 694.4, or about 0x2b6. 
	// although the CPU might struggle to keep up at this speed without a
	// hardware buffer.
	//
	// We'll add the flag for two stop bits.
	// assign	o_uart_setup = 30'h080002b6; // 115200 MBaud @ an 80MHz clock
	assign	o_uart_setup = 30'h0000208d; // 9600 MBaud, 8N1

	initial	o_tx_stb = 1'b0;
	initial	o_tx_data = 8'h00;
	always @(posedge i_clk)
		if ((wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h7)&&(wb_we))
		begin
			o_tx_data <= wb_data[7:0];
			o_tx_stb <= 1'b1;
		end
		else if ((o_tx_stb)&&(~i_tx_busy))
			o_tx_stb <= 1'b0;
	initial	rx_rdy = 1'b0;
	always @(posedge i_clk)
		if (i_rx_stb)
			r_rx_data <= i_rx_data;
	always @(posedge i_clk)
	begin
		if((wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h7)&&(~wb_we))
			rx_rdy <= i_rx_stb;
		else if (i_rx_stb)
			rx_rdy <= (rx_rdy | i_rx_stb);
	end
	assign	o_uart_cts = (~rx_rdy);
	assign	uart_data = { 23'h0, ~rx_rdy, r_rx_data };
	//
	// uart_ack gets returned as part of io_ack, since that happens when
	// io_sel and wb_stb are defined
	//
	// always @(posedge i_clk)
		// uart_ack<= ((wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h7));



	//
	//	FLASH MEMORY CONFIGURATION ACCESS
	//
`ifdef	FLASH_ACCESS
	wbqspiflash #(24)	flashmem(i_clk,
		wb_cyc,(wb_stb)&&(flash_sel),(wb_stb)&&(flctl_sel),wb_we,
			wb_addr[(24-3):0], wb_data,
		flash_ack, flash_stall, flash_data,
		o_qspi_sck, o_qspi_cs_n, o_qspi_mod, o_qspi_dat, i_qspi_dat,
		flash_interrupt);
`else
	assign o_qspi_sck  = 1'b0;
	assign o_qspi_cs_n = 1'b0;
	assign o_qspi_mod  = 2'b0;
	assign o_qspi_dat  = 4'b0;
`endif

	//
	//	MULTIBOOT/ICAPE2 CONFIGURATION ACCESS
	//
	wire	[31:0]	cfg_scope;
`ifdef	FANCY_ICAP_ACCESS
	wbicape6	fpga_cfg(i_clk, wb_cyc,(cfg_sel)&&(wb_stb), wb_we,
				wb_addr[5:0], wb_data,
				cfg_ack, cfg_stall, cfg_data,
				cfg_scope);
`else
	reg	r_cfg_ack;
	always @(posedge i_clk)
		r_cfg_ack <= (wb_cyc)&&(cfg_sel)&&(wb_stb);
	assign	cfg_ack   = r_cfg_ack;
	assign	cfg_stall = 1'b0;
	assign	cfg_data  = 32'h00;
	assign	cfg_scope = 32'h00;
`endif


	//
	//	ON-CHIP RAM MEMORY ACCESS
	//
`ifdef	IMPLEMENT_ONCHIP_RAM
	memdev	#(12) ram(i_clk, wb_cyc, (wb_stb)&&(mem_sel), wb_we,
			wb_addr[11:0], wb_data, wb_sel,
			mem_ack, mem_stall, mem_data);
`else
	assign	mem_data = 32'h00;
	assign	mem_stall = 1'b0;
	reg	r_mem_ack;
	always @(posedge i_clk)
		r_mem_ack <= (wb_cyc)&&(wb_stb)&&(mem_sel);
	assign	mem_ack = r_mem_ack;
`endif

	//
	//
	//	WISHBONE SCOPE
	//
	//
	//
	//
	wire	[31:0]	scop_cfg_data;
	wire		scop_cfg_ack, scop_cfg_stall, scop_cfg_interrupt;
`ifdef	DBG_SCOPE
	wire		scop_cfg_trigger;
	assign	scop_cfg_trigger = (wb_cyc)&&(wb_stb)&&(cfg_sel);
	// wire	scop_trigger = scop_cfg_trigger;
	wire	scop_trigger = (zip_cpu_int) || (cpu_reset);
`ifdef	COMPRESSED_SCOPE
	wbscopc	#(5'ha)
`else
	wbscope	#(5'ha)
`endif
	wbcfgscope(i_clk, 1'b1, scop_trigger,
`ifdef	COMPRESSED_SCOPE
		// cfg_scope[30:0],
		zip_scope_data[30:0],
`else
		// cfg_scope[31:0],
		zip_scope_data[31:0],
`endif
		// Wishbone interface
		i_clk, wb_cyc, (wb_stb)&&(scop_sel),
				wb_we, wb_addr[0], wb_data,
			scop_cfg_ack, scop_cfg_stall, scop_cfg_data,
		scop_cfg_interrupt);
`else
	reg	r_scop_cfg_ack;
	always @(posedge i_clk)
		r_scop_cfg_ack <= (wb_cyc)&&(wb_stb)&&(scop_sel);
	assign	scop_cfg_ack = r_scop_cfg_ack;
	assign	scop_cfg_data = 32'h000;
	assign	scop_cfg_stall= 1'b0;
`endif

	assign	scop_interrupt = scop_cfg_interrupt;
	assign	scop_ack   = scop_cfg_ack;
	assign	scop_stall = scop_cfg_stall;
	assign	scop_data  = scop_cfg_data;

endmodule

