`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: techclub61.su
// Engineer: SY
//
// Create Date:   02:34:29 02/25/2023
// Design Name:   EQA_PATA_host_system
// Module Name:   D:/xilinx_prj/EQA_PATA_host/EQA_PATA_host_test.v
// Project Name:  EQA_PATA_host
// Target Device: XC7A100T-1FGG676
// Tool versions: ISE 14.7
// Description:   Ethernet to PATA adapter
// Current Revision: 005
////////////////////////////////////////////////////////////////////////////////

module EQA_PATA_host_test;

	// Inputs
	reg fpga_gclk;
	reg rst_n;
	reg [3:0] switches;
	reg pata_RESETn;
	reg pata_DMARQ;
	wire pata_DIOWn;
	wire pata_DIORn;
	reg pata_IORDY;
	reg pata_DMACKn;
	reg pata_INTRQ;
	wire [2:0] pata_da;
	wire [1:0] pata_CSn;
	reg e_rxc;
	reg e_rxdv;
	reg e_rxer;
	reg [7:0] e_rxd;
	reg e_txc;

	// Outputs
	wire [7:0] LEDF;
	wire e_gtxc;
	wire e_txen;
	wire e_txer;
	wire [7:0] e_txd;
	wire e_reset;
	wire e_mdc;

	// Bidirs
	wire e_mdio;
	wire [15:0] p0_dd;

	// Vars
	integer i;
	reg [15:0] dd;
	reg data_oe;

	assign p0_dd = (data_oe) ? dd : 16'bz;

	// Instantiate the Unit Under Test (UUT)
	EQA_PATA_host_system uut (
		.fpga_gclk(fpga_gclk), 
		.rst_n(rst_n), 
		.LEDF(LEDF), 
		.switches(switches), 
		.pata_dd(p0_dd), 
		.pata_RESETn(pata_RESETn), 
		.pata_DMARQ(pata_DMARQ), 
		.pata_DIOWn(pata_DIOWn), 
		.pata_DIORn(pata_DIORn), 
		.pata_IORDY(pata_IORDY), 
		.pata_DMACKn(pata_DMACKn), 
		.pata_INTRQ(pata_INTRQ), 
		.pata_da(pata_da), 
		.pata_CSn(pata_CSn), 
		.e_rxc(e_rxc), 
		.e_rxdv(e_rxdv), 
		.e_rxer(e_rxer), 
		.e_rxd(e_rxd), 
		.e_txc(e_txc), 
		.e_gtxc(e_gtxc), 
		.e_txen(e_txen), 
		.e_txer(e_txer), 
		.e_txd(e_txd), 
		.e_reset(e_reset), 
		.e_mdc(e_mdc), 
		.e_mdio(e_mdio)
	);

	initial begin
		// Initialize Inputs
		fpga_gclk = 0;
		rst_n = 0;
		switches = 0;
		pata_RESETn = 0;
		pata_DMARQ = 0;
		pata_IORDY = 0;
		pata_DMACKn = 0;
		pata_INTRQ = 0;
		e_rxc = 0;
		e_rxdv = 0;
		e_rxer = 0;
		e_rxd = 0;
		e_txc = 0;
		data_oe = 0;
		dd = 16'h0000;

		// Wait 100 ns for global reset to finish
		#100;
        
		#100;
		rst_n = 1;
		e_rxc = 1'b0;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxdv = 1;
		e_rxd = 8'h55;
		#10;
		e_rxc = 1'b1;
		for(i = 0; i < 12; i = i + 1) begin
			#10;
			e_rxc = ~e_rxc;
		end
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hD5;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h0A;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h35;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h01;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hFE;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hC0;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hE0;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h4C;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h15;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h20;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h62;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h08;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h45;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h20;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hB0;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hC5;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h40;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h40;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h11;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h08;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hB2;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hC0;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hA8;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h03;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hC0;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hA8;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h02;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h1F;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h92; // port
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h1F;
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h92; // port
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // packet size high
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h16; // packet size low, 22 bytes cmd packet
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h03; // check sum hi
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hFA; // check sum lo
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 000  pata_pkt_num_dummy[7:0]
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h01; // data byte 001  pata_pkt_num_dummy[15:8]
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		// ar_flag_do_read_PIO = pata_flags[3];
		// ar_flag_do_write_PIO = pata_flags[4];
		e_rxd = 8'h08; // data byte 003  pata_flags[7:0] / if pata_flags[0] == 1'b1 then "sense"
		// 08 - do PIO LBA28 read
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 004 pata_flags[15:8]
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h02; // data byte 005 pata_timeout_before[7:0]
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 006 pata_timeout_before[15:8]
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h01; // data byte 007 pata_timeout_after[7:0]
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 008 pata_timeout_after[15:8]
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hA1; // data byte 009 +1 lo
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 010 +1 hi
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hA2; // data byte 011 +2 lo
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 012 +2 hi
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hA3; // data byte 013 +3 lo
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 014 +3 hi
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hA4; // data byte 015 +4 lo
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 016 +4 hi
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hA5; // data byte 017 +5 lo
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 018 +5 hi
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hA6; // data byte 019 +6 lo
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h00; // data byte 020 +6 hi
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'hEC; // data byte 021 +7 lo
		#10;
		e_rxc = 1'b1;
		#10;
		e_rxc = 1'b0;
		e_rxd = 8'h50; // data byte 022 +7 hi - wait mask
		#10;
		e_rxc = 1'b1;
		// ***************
		#10;
		e_rxc = 1'b0;
		e_rxdv = 0;
		e_rxd = 8'h00;

		for(i = 0; i < 200; i = i + 1) begin
			#10 e_rxc = ~e_rxc;
		end

		data_oe = 1;
		dd = 16'h0050;

		for(i = 0; i < 100; i = i + 1) begin
			#10 e_rxc = ~e_rxc;
		end

		data_oe = 0;

		for(i = 0; i < 1000; i = i + 1) begin
			#10 e_rxc = ~e_rxc;
		end

		data_oe = 1;
		dd = 16'h00d0;

		for(i = 0; i < 100; i = i + 1) begin
			#10 e_rxc = ~e_rxc;
		end

		dd = 16'h0050;

		for(i = 0; i < 100; i = i + 1) begin
			#10 e_rxc = ~e_rxc;
		end

		dd = 16'h0058;

		for(i = 0; i < 100; i = i + 1) begin
			#10 e_rxc = ~e_rxc;
		end

		data_oe = 0;

		for(i = 0; i < 15000; i = i + 1) begin
			#10 e_rxc = ~e_rxc;
		end

	end
      
endmodule

