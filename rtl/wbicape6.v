///////////////////////////////////////////////////////////////////////////
//
// Filename:	wbicape6.v
//
// Project:	Wishbone to ICAPE_SPARTAN6 interface conversion
//
// Purpose:	This is a companion project to the ICAPE2 conversion, instead
//		involving a conversion from a 32-bit WISHBONE bus to read
//	and write the ICAPE_SPARTAN6 program.  This is the 'non-simple'
//	portion of the interface, sporting all of the smarts necessary to run
//	the simple interface and make working with ICAPE as simple as 
//	reading and writing from a wishbone bus.  For example, register ID's
//	are given by bus addresses, even though they take extra cycles to 
//	set and clear within the interface.
//
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
///////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015, Gisselquist Technology, LLC
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
///////////////////////////////////////////////////////////////////////////
//
// Following instructions on page 116-117 of the configuration guide ...
//
// W FFFF
// W FFFF
// W AA99
// W 5566
// W 2000	NOOP
// W 2901	Write Type-1 packet header to read register 901??
// W 2000	NOOP
// W 2000	NOOP
// W 2000	NOOP
// W 2000	NOOP
// R vvvv	Read register from previous packet header command
// W 30A1	Write word to CMD register
// W 000D	DESYNC command
// W 2000	NOOP
// W 2000	NOOP
//
// Bits need to be bit-reversed within a byte
//
//
// IPROG example
//
// W FFFF
// W AA99
// W 5566
// W 3261	Write 1 words to GENERAL_1
// W xxxx	write Multiboot start address [15:0]
// W 3281	Write 1 words to GENERAL_2
// W xxxx	write Opcode and multiboot start address [23:16]
// W 32A1	Write 1 words to GENERAL_3
// W xxxx	write Fallback start address
// W 32C1	Write 1 words to GENERAL_4
// W xxxx	write Opcode dne Fallback start address [23:16]
// W 30A1	Write 1 word to CMD
// W 000E	IPGROG Cmd
// W 2000	NOOP
//
//
//
// This fails when using wgregs on the XuLA2 board because the ICAPE port and
// the JTAG port cannot both be active at the same time.
//
//
`define	ICAP_IDLE	5'h0
`define	ICAP_START	5'h1
`define	ICAP_CLOSE	5'hf
module	wbicape6(i_clk, i_wb_cyc, i_wb_stb, i_wb_we, i_wb_addr, i_wb_data,
			o_wb_ack, o_wb_stall, o_wb_data, dbg_data);
	input	i_clk;
	// Wishbone slave inputs
	input			i_wb_cyc, i_wb_stb, i_wb_we;
	input	[5:0]		i_wb_addr;
	input	[31:0]		i_wb_data;
	// Wishbone outputs
	output	reg		o_wb_ack;
	output	reg		o_wb_stall;
	output	wire	[31:0]	o_wb_data;
	output	wire	[31:0]	dbg_data;

	// Interface to the lower level ICAPE port
	reg		icap_cyc, icap_stb, icap_we;
	reg	[15:0]	icap_data_i;
	wire		icap_ack, icap_stall;
	wire	[15:0]	icap_data_o;

	reg	[4:0]	state;
	reg		r_we;
	reg	[15:0]	r_data;

	wire	[25:0]	icap_dbg;
	assign	dbg_data = { i_wb_stb, state[3:0], r_we, icap_dbg };

	reg	stalled_state;
	reg	[7:0]	r_cmd_word;
	wire	[15:0]	w_cmd_word;
	assign	w_cmd_word = { 3'b001, r_cmd_word, 5'h00 }; // Type-1 packet hdr
	initial icap_stb = 1'b0;
	initial icap_cyc = 1'b0;
	initial state    = `ICAP_IDLE;
	always @(posedge i_clk)
	begin
		o_wb_ack   <= 1'b0;
		o_wb_stall <= 1'b0;
		if (stalled_state)
			state <= `ICAP_IDLE;
		else if ((~icap_stall)&&(state != `ICAP_IDLE))
			state <= state + 1;
		case(state)
		`ICAP_IDLE: begin
			icap_stb <= 1'b0;
			icap_cyc <= 1'b0;
			state <= `ICAP_IDLE;
			r_data <= i_wb_data[15:0];
			r_we   <= i_wb_we;
			if ((i_wb_cyc)&&(i_wb_stb))
			begin
				state <= `ICAP_START;
				icap_stb    <= i_wb_stb;
				icap_cyc    <= i_wb_cyc;
				icap_we     <= 1'b1;
				icap_data_i <= 16'hffff;
				r_cmd_word <= { (i_wb_we)? 2'b10:2'b01, i_wb_addr };
				o_wb_stall <= 1'b1;
			end end
		`ICAP_START: begin
			if (~icap_stall)
				icap_data_i <= 16'hffff;
			end
		5'h2: begin
			if (~icap_stall)
				icap_data_i <= 16'haa99;
			end
		5'h3: begin
			if (~icap_stall)
				icap_data_i <= 16'h5566;
			end
		5'h4: begin
			if (~icap_stall)
				icap_data_i <= 16'h2000;
			end
		5'h5: begin
			if (~icap_stall)
			begin
				icap_data_i <= w_cmd_word; // Includes address
			end end
		5'h6: begin // Write
			if (~icap_stall)
			begin 
				if (r_we)
					icap_data_i <= r_data;
				else
					icap_data_i <= 16'h2000;
			end end
		5'h7: begin
			// Need to send four NOOPs before we can begin
			// reading.  Send the four NOOPs for a write anyway.
			if (~icap_stall)
				icap_data_i <= 16'h2000;
			end
		5'h8: begin
			if (~icap_stall)
				icap_data_i <= 16'h2000;
			end
		5'h9: begin
			if (~icap_stall)
			begin
				icap_data_i <= 16'h2000;
				if (r_we)
					state <= `ICAP_CLOSE;
			end end
		5'ha: begin
			if (~icap_stall)
			begin
				// We now request the chip enable line be 
				// dropped, so we can switch from writing to
				// reading
				icap_data_i <= 16'h2000;
				icap_stb    <= 1'b0;
				icap_cyc    <= 1'b0;
			end end
		5'hb: begin
			if (~icap_stall)
			begin
				// Switch the write line to read, must be done
				// w/the chip enable off (hence _cyc=0).  O/w
				// the interface will abort.
				icap_data_i <= 16'h2000;
				icap_we <=1'b0;
			end end
		5'hc: begin
			if (~icap_stall)
			begin
				// We can finally issue our read command
				//	Re-activate the interface, and read
				icap_data_i <= 16'h2000;
				icap_stb    <= 1'b1;
				icap_cyc    <= 1'b1;
			end end
		5'hd: begin
			if (~icap_stall)
			begin
				// De-activate the interface again so we can
				// switch back to write.
				icap_data_i <= 16'h2000;
				icap_stb    <= 1'b0;
				icap_cyc    <= 1'b0;
			end end
		5'he: begin
			if (~icap_stall)
			begin
				// Switch back to write while the interface
				// is deactivated.
				icap_we <= 1'b1;
				icap_data_i <= 16'h2000;
			end end
		`ICAP_CLOSE: begin
			if (~icap_stall)
			begin
				icap_we <= 1'b1;
				// Type 1: Write 1 word to CMD register
				icap_data_i <= 16'h30a1;
				r_data <= icap_data_o;
				icap_stb    <= 1'b1;
				icap_cyc    <= 1'b1;
			end end
		 5'h10: begin // DESYNC Command
			if (~icap_stall)
			begin
				icap_data_i <= 16'h000d;
			end end
		 5'h11: begin // DESYNC must be followed by two NOOPs
			if (~icap_stall)
			begin 
				icap_data_i <= 16'h2000;
			end end
		 5'h12: begin // NOOP
			if (~icap_stall)
			begin
				icap_data_i <= 16'h2000;
				state <= `ICAP_IDLE;
				o_wb_ack <= 1'b1;
				o_wb_stall <= 1'b0;
			end end
		default: begin
			// If we were in the middle of a bus cycle, and got
			// here then ...  we just failed that cycle.  Setting
			// the bus error flag would be appropriate, but we
			// have no such flag to our interface.  Hence we just
			// drop things and depend upon a bus watchdog to 
			// catch that we aren't answering.
			o_wb_ack <= 1'b0;
			o_wb_stall <= 1'b0;
			icap_stb <= 1'b0;
			icap_cyc <= 1'b0;
			state <= `ICAP_IDLE;
			end
		endcase
	end

	wbicapesimple spartancfg(i_clk, icap_cyc,  icap_stb, icap_we,
				icap_data_i,
			icap_ack, icap_stall, icap_data_o,
			icap_dbg);


	assign	o_wb_data = { 16'h0000, r_data };

	reg	[4:0]	last_state;
	initial	last_state = `ICAP_IDLE;
	always @(posedge i_clk)
		last_state <= state;

	reg	[11:0]	reset_ctr;
	always @(posedge i_clk)
		if (last_state != state)
			reset_ctr <= 0;
		else if (state == `ICAP_IDLE)
			reset_ctr <= 0;
		else
			reset_ctr <= reset_ctr + 1;
	always @(posedge i_clk)
		stalled_state <= (&reset_ctr);
	
endmodule
