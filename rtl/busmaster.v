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
// 2646 Slice LUTs after translate becomes 2324 once built.
// After i_rst simplification,			2617 LUTs ->2308
// After adding OPT_CIS, 			2719 LUTs ->23??
// After dropping scope holdoff to 9 bits,	2698 LUTs ->23??
// After dropping scope size from 10 to 6 bits,	2685 LUTs -> 
// After simplifying scope some more,		2684 LUTs -> (WONT FIT)
// Removed scope--wouldn't fit
// Removed OPT_CIS ... should be back to what it was before, 2582 LUTs -> ??
//
// Starting again with current state, 2579 LUTS -> 2103
//	Becomes 2681 -> 2187
//
`include "builddate.v"
//
`define	IMPLEMENT_ONCHIP_RAM
`define	FLASH_ACCESS
// `define	DBG_SCOPE	// About 204 LUTs, at 2^6 addresses
// `define	COMPRESSED_SCOPE
`define	INCLUDE_CPU_RESET_LOGIC
module	busmaster(i_clk, i_rst,
		i_rx_stb, i_rx_data, o_tx_stb, o_tx_data, i_tx_busy,
			o_uart_rts_n,
		// The SPI Flash lines
		o_qspi_cs_n, o_qspi_sck, o_qspi_dat, i_qspi_dat, o_qspi_mod,
		// The board I/O
		i_btn, o_led, o_pwm, o_pwm_aux,
		// Keypad connections
		i_kp_row, o_kp_col,
		// GPIO lines
		i_gpio, o_gpio);
	parameter	BUS_ADDRESS_WIDTH=23,
			ZIP_ADDRESS_WIDTH=BUS_ADDRESS_WIDTH,
			CMOD_ZIPCPU_RESET_ADDRESS=32'h1200000;
	localparam	ZA=ZIP_ADDRESS_WIDTH,
			BAW=BUS_ADDRESS_WIDTH; // 24bits->2,258,23b->2181
	// 2^14 bytes requires a LGMEMSZ of 14, and 12 address bits ranging from
	// 0 to 11.  As with many other devices, the wb_cyc line is more for
	// form than anything else--it is ignored by the memory itself.
	localparam	LGMEMSZ=14;	// Takes 8 BLKRAM16 elements for 14
	// As with the memory size, the flash size is also measured in log_2 of
	// the number of bytes.
	localparam	LGFLASHSZ = 24;
	input			i_clk, i_rst;
	input			i_rx_stb;
	input		[7:0]	i_rx_data;
	output	reg		o_tx_stb;
	output	reg	[7:0]	o_tx_data;
	input			i_tx_busy;
	output	wire		o_uart_rts_n;
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
	wire	[3:0]		io_addr;
	assign	io_addr = {
			wb_addr[(LGFLASHSZ-2)],	// Flash
			wb_addr[(LGMEMSZ-2)],	// RAM
			wb_addr[ 9],		// SCOPE
			wb_addr[ 8] };		// I/O

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

	wire	cpu_reset, watchdog_int;
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
`ifdef	INCLUDE_CPU_RESET_LOGIC
	reg	btn_reset, x_button, r_button;
	initial	btn_reset = 1'b0;
	initial	x_button = 1'b0;
	initial	r_button = 1'b0;
	always @(posedge i_clk)
	begin
		x_button <= i_btn[1];
		r_button <= x_button;
		btn_reset <= ((r_button)&&(zip_cpu_int))||(watchdog_int);
	end
	assign	cpu_reset = btn_reset;
`else
	assign	cpu_reset = 1'b0;
`endif

	zipbones #(CMOD_ZIPCPU_RESET_ADDRESS,ZA,6)
		swic(i_clk, btn_reset, // 1'b0,
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

	wire	io_sel, flash_sel, flctl_sel, scop_sel, mem_sel,
			none_sel, many_sel;
	wire	flash_ack, scop_ack, cfg_ack, mem_ack, many_ack;
	wire	io_stall, flash_stall, scop_stall, cfg_stall, mem_stall;
	reg	io_ack;

	wire	[31:0]	flash_data, scop_data, cfg_data, mem_data, pwm_data,
			spio_data, gpio_data, uart_data;
	reg	[31:0]	io_data;
	reg	[(BAW-1):0]	bus_err_addr;

	assign	wb_ack = (wb_cyc)&&((io_ack)||(scop_ack)
				||(mem_ack)||(flash_ack)||((none_sel)&&(1'b1)));
	assign	wb_stall = ((io_sel)&&(io_stall))
			||((scop_sel)&&(scop_stall))
			||((mem_sel)&&(mem_stall))
			||((flash_sel||flctl_sel)&&(flash_stall));
			// (none_sel)&&(1'b0)

	assign	wb_idata =  (io_ack|scop_ack)?((io_ack )? io_data  : scop_data)
			: ((mem_ack)?(mem_data)
			: flash_data);
	assign	wb_err = ((wb_stb)&&(none_sel || many_sel)) || many_ack;

	// Addresses ...
	//	0000 xxxx	configuration/control registers
	//	1 xxxx xxxx xxxx xxxx xxxx	Up-sampler taps
	assign	io_sel   =((wb_cyc)&&(io_addr[3:0]==4'h1));
	assign	scop_sel =((wb_cyc)&&(io_addr[3:1]==3'h1));
	assign	flctl_sel= 1'b0; // ((wb_cyc)&&(io_addr[5:1]==5'h1));
	assign	mem_sel  =((wb_cyc)&&(io_addr[3:2]==2'h1));
	assign	flash_sel=((wb_cyc)&&(io_addr[3]));

	assign	many_ack = 1'b0;
	assign	many_sel = 1'b0;
	assign	none_sel =((wb_stb)&&(io_addr==4'h0));

	wire		flash_interrupt, scop_interrupt, timer_int,
			gpio_int, pwm_int, keypad_int,button_int;


	//
	//
	//
	reg		rx_rdy;
	wire	[10:0]	int_vector;
	assign	int_vector = { 
					gpio_int, pwm_int, keypad_int,
				(~o_tx_stb), rx_rdy,
				1'b0, timer_int,
				1'b0, scop_interrupt,
				wb_err, button_int };

	wire	[31:0]	pic_data;
	icontrol #(11)	pic(i_clk, 1'b0, (wb_stb)&&(io_sel)
					&&(wb_addr[3:0]==4'h0)&&(wb_we),
			wb_data, pic_data, int_vector, w_interrupt);

	initial	bus_err_addr = 0; // `DATESTAMP;
	always @(posedge i_clk)
		if (wb_err)
			bus_err_addr <= wb_addr;

	wire	[31:0]	timer_data, watchdog_data;
	wire		zta_ack, zta_stall, ztb_ack, ztb_stall;
	ziptimer	#(32,31,1)
		thetimer(i_clk, 1'b0, 1'b1, wb_cyc,
				(wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h2),
				wb_we, wb_data, zta_ack, zta_stall, timer_data,
				timer_int);
	ziptimer	#(32,31,0)
		watchdog(i_clk, cpu_reset, 1'b1, wb_cyc,
				(wb_stb)&&(io_sel)&&(wb_addr[3:0]==4'h3),
				wb_we, wb_data, ztb_ack, ztb_stall, watchdog_data,
				watchdog_int);

	always @(posedge i_clk)
		case(wb_addr[3:0])
			4'h0: io_data <= pic_data;
			4'h1: io_data <= { {(32-BAW){1'b0}}, bus_err_addr };
			4'h2: io_data <= timer_data;
			4'h3: io_data <= watchdog_data;
			4'h4: io_data <= pwm_data;
			4'h5: io_data <= spio_data;
			4'h6: io_data <= gpio_data;
			4'h7: io_data <= uart_data;
			default: io_data <= `DATESTAMP;
			// 4'h8: io_data <= `DATESTAMP;
		endcase
	always @(posedge i_clk)
		io_ack <= (wb_stb)&&(io_sel);
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
	assign	o_uart_rts_n = (rx_rdy);
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
	wbqspiflash #(LGFLASHSZ)
		flashmem(i_clk,
			wb_cyc,(wb_stb)&&(flash_sel),(wb_stb)&&(flctl_sel),
			wb_we, wb_addr[(LGFLASHSZ-3):0], wb_data,
			flash_ack, flash_stall, flash_data,
			o_qspi_sck, o_qspi_cs_n, o_qspi_mod,
				o_qspi_dat, i_qspi_dat,
			flash_interrupt);
`else
	assign o_qspi_sck  = 1'b0;
	assign o_qspi_cs_n = 1'b0;
	assign o_qspi_mod  = 2'b0;
	assign o_qspi_dat  = 4'b0;
`endif

	//
	//	ON-CHIP RAM MEMORY ACCESS
	//
`ifdef	IMPLEMENT_ONCHIP_RAM
	memdev	#(.LGMEMSZ(LGMEMSZ))
		ram(i_clk, wb_cyc, (wb_stb)&&(mem_sel), wb_we,
			wb_addr[(LGMEMSZ-3):0], wb_data, wb_sel,
			mem_ack, mem_stall, mem_data);
`else
	assign	mem_data = 32'h00;
	assign	mem_stall = 1'b0;
	reg	r_mem_ack;
	always @(posedge i_clk)
		r_mem_ack <= (wb_stb)&&(mem_sel);
	assign	mem_ack = r_mem_ack;
`endif

	//
	//
	//	WISHBONE SCOPE
	//
	//
	//
	//
	wire	[31:0]	scop_cpu_data;
	wire		scop_cpu_ack, scop_cpu_stall, scop_cpu_interrupt;
`ifdef	DBG_SCOPE
	wire	scop_trigger = (zip_cpu_int) || (cpu_reset);
`ifdef	COMPRESSED_SCOPE
	wbscopc	#(5'ha)
`else
	wbscope	#(.LGMEM(5'h6), .HOLDOFFBITS(9))
`endif
	cpuscope(i_clk, 1'b1, scop_trigger,
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
			scop_cpu_ack, scop_cpu_stall, scop_cpu_data,
		scop_cpu_interrupt);
`else
	reg	r_scop_cpu_ack;
	always @(posedge i_clk)
		r_scop_cpu_ack <= (wb_stb)&&(scop_sel);
	assign	scop_cpu_ack = r_scop_cpu_ack;
	assign	scop_cpu_data = 32'h000;
	assign	scop_cpu_stall= 1'b0;
`endif

	assign	scop_interrupt = scop_cpu_interrupt;
	assign	scop_ack   = scop_cpu_ack;
	assign	scop_stall = scop_cpu_stall;
	assign	scop_data  = scop_cpu_data;

endmodule

