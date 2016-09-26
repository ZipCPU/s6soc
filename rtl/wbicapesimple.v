///////////////////////////////////////////////////////////////////////////
//
// Filename:	wbicapesimple.v
//
// Project:	Wishbone to ICAPE_SPARTAN6 interface conversion
//
// Purpose:	This is a companion project to the ICAPE2 conversion, instead
//		involving a conversion from a 32-bit WISHBONE bus to read
//	and write the ICAPE_SPARTAN6 program.  Since this is a simple interface
//	only, the smarts have been stripped out of it.  Therefore, if you wish
//	to write to or read from particular registers, you will need to do all
//	the sequencing yourself.
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
module	wbicapesimple(i_clk,
		i_wb_cyc, i_wb_stb, i_wb_we, i_wb_data,
			o_wb_ack, o_wb_stall, o_wb_data, icap_dbg);
	input			i_clk;
	// Wishbone inputs
	input			i_wb_cyc, i_wb_stb, i_wb_we;
	input		[15:0]	i_wb_data;
	// Wishbone outputs
	output	reg		o_wb_ack;
	output	wire		o_wb_stall;
	output	reg	[15:0]	o_wb_data;
	output	wire	[25:0]	icap_dbg;


	// Divide our clock by 8.  Thus, a 100 MHz clock with a 10 ns period
	// gets divided down to an 80 ns period, which is greater than the 
	// minimum 50 ns as per the data sheet.
	reg	[2:0]	clk_divider;
	initial	clk_divider=0;
	always @(posedge i_clk)
		clk_divider <= clk_divider + 3'h1;

	reg	icap_posedge, icap_preedge;
	initial	icap_posedge = 0;
	always @(posedge i_clk)
		icap_posedge <= (clk_divider == 3'b011);
	initial	icap_preedge = 0;
	always @(posedge i_clk)
		icap_preedge <= (clk_divider == 3'b010);
		// icap_posedge <= (&clk_divider);

	wire	icap_clk;
	assign	icap_clk = clk_divider[2];

	reg		icap_ce_n, icap_we_n;
	reg	[15:0]	icap_data_i;


	wire		icap_busy;
	wire	[15:0]	icap_data_o;
	ICAP_SPARTAN6	icap_inst(
		.BUSY(icap_busy), // Active high, low during all writes
		.O(icap_data_o),
		.CE(icap_ce_n),
		.CLK(icap_clk),
		.I(icap_data_i),
		.WRITE(icap_we_n));

	// assign	o_wb_stall = (i_wb_cyc)&&((~icap_posedge)||((~icap_ce_n)&&(icap_busy)));

	wire	[15:0]	brev_i_wb_data;
	assign	brev_i_wb_data[ 0] = i_wb_data[ 7];
	assign	brev_i_wb_data[ 1] = i_wb_data[ 6];
	assign	brev_i_wb_data[ 2] = i_wb_data[ 5];
	assign	brev_i_wb_data[ 3] = i_wb_data[ 4];
	assign	brev_i_wb_data[ 4] = i_wb_data[ 3];
	assign	brev_i_wb_data[ 5] = i_wb_data[ 2];
	assign	brev_i_wb_data[ 6] = i_wb_data[ 1];
	assign	brev_i_wb_data[ 7] = i_wb_data[ 0];
	//
	assign	brev_i_wb_data[ 8] = i_wb_data[15];
	assign	brev_i_wb_data[ 9] = i_wb_data[14];
	assign	brev_i_wb_data[10] = i_wb_data[13];
	assign	brev_i_wb_data[11] = i_wb_data[12];
	assign	brev_i_wb_data[12] = i_wb_data[11];
	assign	brev_i_wb_data[13] = i_wb_data[10];
	assign	brev_i_wb_data[14] = i_wb_data[ 9];
	assign	brev_i_wb_data[15] = i_wb_data[ 8];
	//

	initial	icap_data_i = 0;
	always @(posedge i_clk)
		if (icap_preedge)
		begin
			if (~i_wb_cyc)
				icap_data_i <= 16'hffff;
			else if ((icap_ce_n)||(~icap_busy))
				icap_data_i <= brev_i_wb_data[15:0];
		end

	initial	icap_we_n = 1'b1;
	always @(posedge i_clk)
		if ((icap_preedge)&&((icap_ce_n)||(~icap_busy)))
			icap_we_n <= ~i_wb_we;
	initial	icap_ce_n = 1'b1;
	always @(posedge i_clk)
		if ((icap_preedge)&&((icap_ce_n)||(~icap_busy)))
			icap_ce_n <= ~((i_wb_cyc)&&(i_wb_stb));
		else if ((icap_preedge)&&(~i_wb_cyc))
			icap_ce_n <= 1'b1;
	//initial	r_wb_stall = 1'b0;
	//always @(posedge i_clk)
		//r_wb_stall <= (~icap_posedge)&&(
					//&&(icap_preedge)&&(~icap_busy));
	// assign	o_wb_stall = (i_wb_cyc)&&((~icap_posedge)||((~icap_ce_n)&&(icap_busy)));
	assign	o_wb_stall = (~icap_posedge)
				||((i_wb_cyc)&&(~icap_ce_n)&&(icap_busy));
	initial	o_wb_ack = 1'b0;
	always @(posedge i_clk)
		o_wb_ack <= ((~icap_ce_n)&&(i_wb_cyc)
					&&(icap_preedge)&&(~icap_busy));

	wire	[15:0]	brev_icap_data;
	assign	brev_icap_data[ 0] = icap_data_o[ 7];
	assign	brev_icap_data[ 1] = icap_data_o[ 6];
	assign	brev_icap_data[ 2] = icap_data_o[ 5];
	assign	brev_icap_data[ 3] = icap_data_o[ 4];
	assign	brev_icap_data[ 4] = icap_data_o[ 3];
	assign	brev_icap_data[ 5] = icap_data_o[ 2];
	assign	brev_icap_data[ 6] = icap_data_o[ 1];
	assign	brev_icap_data[ 7] = icap_data_o[ 0];
	//
	assign	brev_icap_data[ 8] = icap_data_o[15];
	assign	brev_icap_data[ 9] = icap_data_o[14];
	assign	brev_icap_data[10] = icap_data_o[13];
	assign	brev_icap_data[11] = icap_data_o[12];
	assign	brev_icap_data[12] = icap_data_o[11];
	assign	brev_icap_data[13] = icap_data_o[10];
	assign	brev_icap_data[14] = icap_data_o[ 9];
	assign	brev_icap_data[15] = icap_data_o[ 8];
	//
	initial	o_wb_data = 16'h000;
	always @(posedge i_clk)
		if ((icap_posedge)&&((icap_ce_n)||(~icap_busy)))
			o_wb_data <= brev_icap_data;

	assign	icap_dbg[25:0] = {
			i_wb_cyc,i_wb_stb, i_wb_we, o_wb_ack, o_wb_stall,
			icap_posedge, icap_clk, icap_ce_n, icap_busy, icap_we_n,
			(icap_we_n)?icap_data_o : icap_data_i };

endmodule

