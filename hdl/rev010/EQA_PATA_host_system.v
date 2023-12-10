`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: techclub61.su
// Engineer: SY
// 
// Create Date:    02:23:38 02/25/2023 
// Design Name:    EQA_PATA_host
// Module Name:    EQA_PATA_host_system 
// Project Name:   2312_Ehernet_Qt_ATA
// Target Devices: XC7A100T-1fgg676 Wukong
// Tool versions:  ISE 14.7
// Description:    PATA host controlled via ethernet UDP
// SPI Flash:      S25FL128L
//
// Revision: k23.03.27.001 - Start of simulation
// Revision: k23.04.10.002 - First command have sent to HDD
// Revision: k23.04.12.003 - TX1 buffer for PIO read added
// Revision: k23.04.26.004 - PIO data read cycle from +0 added
// Revision: k23.05.11.005 - 1 sector PIO data read fixed and work well on ID command
// Revision: k23.06.15.006 - 1 sector PIO data write first try
// Revision: k23.06.19.007 - ata_idle_cnt counter added
// Revision: k23.07.03.008 - logic for ar_flag_skip_cmd added in case PIO data R/W
// Revision: k23.08.08.009 - write buffer 1 loop back test
// Revision: k23.09.27.010 - simple HDD power control 
//////////////////////////////////////////////////////////////////////////////////
module EQA_PATA_host_system(
   /* system signal*/
   input              fpga_gclk,
   input              rst_n,
	output reg [1:0]   power_sw,
	input [3:0]        switches,
   /* PATA host port 0 */
   inout [15:0]       pata_dd,
   output reg         pata_RESETn,
   input              pata_DMARQ,
   output reg         pata_DIOWn,
   output reg         pata_DIORn,
   input              pata_IORDY,
   input              pata_DMACKn,
   input              pata_INTRQ,
   output reg [2:0]   pata_da,
   output reg [1:0]   pata_CSn,
   /* rx interface */
   input              e_rxc,   //125Mhz
   input              e_rxdv,
   input              e_rxer,
   input [7:0]        e_rxd,
   /* tx interface */
   input              e_txc,
   output             e_gtxc,
   output             e_txen,
   output reg         e_txer,
   output [7:0]       e_txd,
   /* PHY setting interface */
   output             e_reset,
   output             e_mdc,
   inout              e_mdio
    );

wire [15:0]     pata_version;
assign pata_version = 16'd10;

//TX counter
localparam	
   PRE_CNT  = 9,
   RAM_CNT  = 76,
   RAM1_CNT  = 1080, // TX1_FSM
	RAM_CPY_CNT = 75,
	RAM_FCNT = 80,
	RAM1_FCNT = 1084; // TX1_FSM

//+ common for  and TX1_FSM
//+TX_FSM state definition
localparam	
   IDLE         = 4'b0000, // 0
   STOR_SW1     = 4'b0001, // 1
   STOR_SW2     = 4'b0010, // 2
   STOR_SW3     = 4'b0011, // 3
   CALC_CRC     = 4'b0100, // 4
   SEND_CRC     = 4'b0101, // 5
   SEND_CRC1    = 4'b0110, // 6
   SEND_CRC2    = 4'b0111, // 7
   SEND_CRC3    = 4'b1000, // 8
   SEND_CRC4    = 4'b1001, // 9
	START_TX     = 4'b1010, // a
	SEND_DATA    = 4'b1011, // b
   IDLE_CODE    = 4'b1100; // c
//-TX_FSM state definition

//+TX1_FSM state definition              ***
localparam	
   IDLE1        = 5'b00000, // 0
   STOR1_SET1   = 5'b00001, // 1
   STOR1_CP_HDR = 5'b00010, // 2
   STOR1_WAIT_W = 5'b00011, // 3
   STOR1_CPY_D1 = 5'b00100, // 4
   STOR1_CPY_D2 = 5'b00101, // 5
   STOR1_CHKFIN = 5'b00110, // 6
   STOR1_ZADR   = 5'b00111, // 7
   CALC1_CRC    = 5'b01000, // 8
   SEND1_CRC    = 5'b01001, // 9
   SEND1_CRC1   = 5'b01010, // a
   SEND1_CRC2   = 5'b01011, // b
   SEND1_CRC3   = 5'b01100, // c
   SEND1_CRC4   = 5'b01101, // d
	START1_TX    = 5'b01110, // e
	SEND1_DATA   = 5'b01111, // f
   IDLE1_CODE   = 5'b10000; // 10
//-TX1_FSM state definition              ***

reg [3:0]      tx_state;         /*synthesis preserve*/
reg [3:0]      tx_nxt_state;     /*synthesis preserve*/
reg [4:0]      tx1_state;        /*synthesis preserve*/
reg [4:0]      tx1_nxt_state;    /*synthesis preserve*/

reg            restart; // ???

reg            tx_data1; // if 1 then enable data tx or enable sense tx 

reg [6:0]      ram_addr;
reg            ram_wren;
wire [7:0]     ram_data;
reg [7:0]      ram_data_r;
reg [7:0]      ram_data_in;
reg            txen;
reg [7:0]      txd;

wire [31:0]    crc;
reg            crcen;
reg            crcrst;

reg [10:0]     ram1_addr;      // TX1_FSM
reg            ram1_wren;      // TX1_FSM
wire [7:0]     ram1_data;      // TX1_FSM
reg [7:0]      ram1_data_r;    // TX1_FSM
reg [7:0]      ram1_data_in;   // TX1_FSM
reg            txen1;          // TX1_FSM
reg [7:0]      txd1;           // TX1_FSM
reg            tx_hdr_cp_flag; // TX1_FSM
reg            tx2ata_sectfin; // TX1_FSM
reg            ata2tx_txfin;   // TX1_FSM

wire [31:0]    crc1;
reg            crcen1;
reg            crcrst1;

// UDP TX packet counter
reg [31:0]     pkt_cnt;
// ATA regs to UDP
reg [15:0]     pata_pkt_num;
reg [15:0]     pata_flags;
reg [15:0]     pata_timeout_before;
reg [15:0]     pata_timeout_after;
reg [15:0]     pata_p1;
reg [15:0]     pata_p2;
reg [15:0]     pata_p3;
reg [15:0]     pata_p4;
reg [15:0]     pata_p5;
reg [15:0]     pata_p6;
reg [15:0]     pata_p7;
reg [7:0]      pata_p7rd;
reg [15:0]     pata_p0rd;
wire [7:0]     pata_flags_tail;
//
reg            rx_atacmd_flag;     // command resived via UDP and start to send to HDD
reg            rx_atasense_flag;   // tx sense sector has to be sent
reg            rx_testbuf1_flag;   // loop back for write buffer 1
reg            atacmd_done_flag;
reg [15:0]     pata_pkt_num_dummy;
reg [15:0]     pata_flags_dummy;
reg [15:0]     pata_flags_return;
reg            ata_read3_cyc_finish;  // +0 read cycle finish flag, used in ATA FSM and TX1_FSM
reg            power_sw_trig; // simple power control bit
//+ rx buffer var
reg            ar_flag_fill_buffer;
reg [9:0]      rx_buf1_addr;
reg [9:0]      pata_buf1_addr;
reg            rx_buf1_we;
wire [7:0]     pata_buf1_data;
reg [15:0]     pata_cmd_pkt_num;
reg [7:0]      pata_buf_fill_num;
reg [15:0]     pata_cmd_sect_num;
reg            rx_buf1_fill_done;
reg            rx_buf1_fill_reset;
reg            rx_buf1_filled;
//- rx buffer var

localparam
   R_IDLE       = 4'b0000, 
   R_PRE        = 4'b0010,
   R_MAC        = 4'b0110,
   R_HEADER     = 4'b0111,
   R_DATA       = 4'b0101,
   R_DATA_S     = 4'b0001,
   R_DATA_FIN   = 4'b0011,
   R_FIHISH     = 4'b1000;

reg [3:0]      rx_state;      /*synthesis preserve*/
reg [3:0]      rx_nxt_state;  /*synthesis preserve*/

localparam DST_MAC   = 48'h000a3501fec0;
localparam DST_ADDR  = 32'hc0a80002; /* 192.168.0.2 */
localparam DST_PORT  = 16'd8082;     /* 8082 for ATA op. */
localparam PRESEMBLE = 8'h55;
localparam PRESTART  = 8'hd5;
localparam IP_TYPE   = 16'h0800;
localparam UDP_TYPE  = 8'h11;

reg [15:0]     rx_cnt;
reg [10:0]     rx_data_cnt; // 11 bit cnt
reg [159:0]    rx_ip_header;
reg [63:0]     rx_udp_header;
reg [111:0]    rx_mac;
wire [47:0]    rx_dst_mac_w;
wire [31:0]    rx_dst_addr_w;
wire [15:0]    rx_dst_port_w;
wire [7:0]     rx_ip_type;
reg [15:0]     rx_data_len;
reg            rx_update;

wire           is_rxd_PRE;
wire           is_prestart;
wire           is_cur_state_n;

localparam
   R_PRE_CNT  = 7,
   R_MAC_CNT  = 14,
   R_HRD_CNT  = 28,
   R_CRC_CNT  = 4,
   R_CODE_CNT = 12;

wire           flag_pre_cnt;
wire           flag_mac_over;
wire           flag_header_idle;
wire           flag_header_over;
wire           flag_data_idle;
wire           flag_data_over;

//////////////////////////////////////////////////////////////////////////////////

assign e_gtxc = e_rxc; //125Mhz

assign e_reset = 1'b1;
assign e_mdio = 1'bz;
assign e_mdc = 1'b0;

assign flag_ram_over  = (ram_addr >= RAM_CNT - 16'd1)  ? 1'b1 : 1'b0;
assign flag_cpy_over  = (ram_addr >= RAM_CPY_CNT - 16'd1)  ? 1'b1 : 1'b0;
assign flag_fram_over = (ram_addr >= RAM_FCNT - 16'd1) ? 1'b1 : 1'b0;
assign flag_pre_over  = (ram_addr >= PRE_CNT - 16'd1)  ? 1'b1 : 1'b0;

assign flag_ram_over1  = (ram1_addr >= RAM1_CNT - 16'd1)  ? 1'b1 : 1'b0; // TX1_FSM
assign flag_fram_over1 = (ram1_addr >= RAM1_FCNT - 16'd1) ? 1'b1 : 1'b0; // TX1_FSM
assign flag_pre_over1  = (ram1_addr >= PRE_CNT - 16'd1)  ? 1'b1 : 1'b0;  // TX1_FSM

//+ TX_FSM reset and state copy
always @ (posedge e_rxc or negedge rst_n) // TX FSM main
begin
	if (!rst_n) begin
		tx_state	<=	IDLE;
	end else begin
		tx_state	<=	tx_nxt_state;
	end
end
//- TX_FSM reset and state copy

//+ TX1_FSM reset and state copy         ***
always @ (posedge e_rxc or negedge rst_n)
begin
	if (!rst_n) begin
		tx1_state	<=	IDLE1; // TX1_FSM
	end else begin
		tx1_state	<=	tx1_nxt_state;
	end
end
//- TX1_FSM reset and state copy         ***

//+ TX_FSM state selection
always @ (*)
begin
	tx_nxt_state = tx_state;
	case (tx_state)
	IDLE:
		if (rx_atacmd_flag || rx_atasense_flag || atacmd_done_flag)
			tx_nxt_state = STOR_SW1;

	STOR_SW1:
		tx_nxt_state = STOR_SW2;
	STOR_SW2:
		if(flag_cpy_over)
			tx_nxt_state = STOR_SW3;
	STOR_SW3:
		tx_nxt_state = CALC_CRC;
			
	CALC_CRC:
		if (flag_ram_over)	
			tx_nxt_state = SEND_CRC;
			
	SEND_CRC:
		tx_nxt_state = SEND_CRC1;
	SEND_CRC1:
		tx_nxt_state = SEND_CRC2;
	SEND_CRC2:
		tx_nxt_state = SEND_CRC3;
	SEND_CRC3:
		tx_nxt_state = SEND_CRC4;
	SEND_CRC4:
		tx_nxt_state = START_TX;

	START_TX:
		tx_nxt_state = SEND_DATA;
		
	SEND_DATA:
		if (flag_fram_over)	
			tx_nxt_state = IDLE_CODE;

	IDLE_CODE:
		tx_nxt_state = IDLE;

	default: tx_nxt_state = IDLE;
	endcase
end
//- TX_FSM state selection

//+ TX1_FSM state selection              ***
always @ (*)
begin
	tx1_nxt_state = tx1_state;
	case (tx1_state)
	IDLE1: // TX1_FSM
		if (ata_udp_data_tx_in_progress)
			tx1_nxt_state = STOR1_SET1;

	STOR1_SET1: // set ram address  // TX1_FSM
		tx1_nxt_state = STOR1_CP_HDR;

	STOR1_CP_HDR: // TX1_FSM
		if(tx_hdr_cp_flag)
			tx1_nxt_state = STOR1_WAIT_W;

	STOR1_WAIT_W: // wait for data enable for copy here // TX1_FSM
		if( (ata_read3_cyc_finish) || (tx2ata_sectfin) ) begin
			tx1_nxt_state = STOR1_CPY_D1;
		end

	STOR1_CPY_D1: // TX1_FSM
		tx1_nxt_state = STOR1_CPY_D2;

	STOR1_CPY_D2: // TX1_FSM
		tx1_nxt_state = STOR1_CHKFIN;

	STOR1_CHKFIN: // TX1_FSM
		if(tx2ata_sectfin) // check for 512 bytes receive flag
			tx1_nxt_state = STOR1_ZADR;
		else
			tx1_nxt_state = STOR1_WAIT_W;

	STOR1_ZADR: // TX1_FSM
		tx1_nxt_state = CALC1_CRC;

	CALC1_CRC: // TX1_FSM
		if (flag_ram_over1)
			tx1_nxt_state = SEND1_CRC;
			
	SEND1_CRC: // TX1_FSM
		tx1_nxt_state = SEND1_CRC1;
	SEND1_CRC1: // TX1_FSM
		tx1_nxt_state = SEND1_CRC2;
	SEND1_CRC2: // TX1_FSM
		tx1_nxt_state = SEND1_CRC3;
	SEND1_CRC3: // TX1_FSM
		tx1_nxt_state = SEND1_CRC4;
	SEND1_CRC4: // TX1_FSM
		tx1_nxt_state = START1_TX;

	START1_TX: // TX1_FSM
		tx1_nxt_state = SEND1_DATA;
		
	SEND1_DATA: // TX1_FSM
		if (flag_fram_over1)	
			tx1_nxt_state = IDLE1_CODE;

	IDLE1_CODE: // TX1_FSM
		tx1_nxt_state = IDLE1;

	default: tx1_nxt_state = IDLE1;
	endcase
end
//- TX1_FSM state selection              ***

//+ TX_FSM crc + ram_wren + ram_addr
always @ (posedge e_rxc)
begin
	case (tx_state)
	IDLE:
	begin
		crcen	<=	1'b0;
		crcrst	<=	1'b1;
	   ram_wren <= 1'b0;
		ram_addr <= 0;
	end

	STOR_SW1:
	begin
	   ram_wren <= 1'b1;
		ram_addr <= 7'd50; // begin of UDP data
	end

	STOR_SW2:
	begin
	   ram_wren <= 1'b1;
      ram_addr <= ram_addr + 1'b1;
	end

	STOR_SW3:
	begin
	   ram_wren <= 1'b0;
		ram_addr <= 0;
	end

	CALC_CRC:
	begin
      ram_addr <= ram_addr + 1'b1;
		crcrst	<=	1'b0;
		if (flag_pre_over)
			crcen	<=	1'b1;
	end
	
	SEND_CRC:
	begin
		crcen	<=	1'b0;
	end

	SEND_CRC1:
	begin
	   ram_wren <= 1'b1;
	end
	
	SEND_CRC2, SEND_CRC3:
	begin
	   ram_wren <= 1'b1;
      ram_addr <= ram_addr + 1'b1;
	end

	SEND_CRC4:
	begin
	   ram_wren <= 1'b1;
      ram_addr <= ram_addr + 1'b1;
	end
	
	START_TX:
	begin
	   crcrst <= 1'b1;
	   ram_wren <= 1'b0;
		ram_addr <= 0;
	end
	
	SEND_DATA:
	begin
      ram_addr <= ram_addr + 1'b1;
	end
	
	default: ;
	endcase
end
//- TX_FSM crc + ram_wren + ram_addr

//+ TX1_FSM crc + ram_wren + ram_addr    ***
always @ (posedge e_rxc)
begin
	case (tx1_state)
	IDLE1: // TX1_FSM
	begin
		crcen1    <= 1'b0;
		crcrst1   <= 1'b1;
	   ram1_wren <= 1'b0;
		ram1_addr <= 0;
	end

	STOR1_SET1: // TX1_FSM
	begin
		ram1_addr <= 11'd49; // begin of UDP data - 1
	end

	STOR1_CP_HDR: // TX1_FSM
	begin
		ram1_wren <= 1'b1;
		ram1_addr <= ram1_addr + 1'b1;
	end

	STOR1_WAIT_W: // TX1_FSM
	begin
		ram1_wren <= 1'b0;
	end

	STOR1_CPY_D1: // TX1_FSM
	begin
		ram1_wren <= 1'b1;
	end

	STOR1_CPY_D2: // TX1_FSM
	begin
		ram1_wren <= 1'b1;
		ram1_addr <= ram1_addr + 1'b1;
	end

	STOR1_CHKFIN: // TX1_FSM
	begin
		ram1_addr <= ram1_addr + 1'b1;
		ram1_wren <= 1'b0;
	end

	STOR1_ZADR: // TX1_FSM
	begin
		ram1_addr <= 0;
	end

	CALC1_CRC: // TX1_FSM
	begin
		ram1_addr <= ram1_addr + 1'b1;
		crcrst1 <= 1'b0;
		if (flag_pre_over1)
			crcen1 <= 1'b1;
	end
	
	SEND1_CRC: // TX1_FSM
	begin
		crcen1 <= 1'b0;
	end

	SEND1_CRC1: // TX1_FSM
	begin
	   ram1_wren <= 1'b1;
	end
	
	SEND1_CRC2, SEND1_CRC3: // TX1_FSM
	begin
	   ram1_wren <= 1'b1;
      ram1_addr <= ram1_addr + 1'b1;
	end

	SEND1_CRC4: // TX1_FSM
	begin
	   ram1_wren <= 1'b1;
      ram1_addr <= ram1_addr + 1'b1;
	end
	
	START1_TX: // TX1_FSM
	begin
	   crcrst1 <= 1'b1;
	   ram1_wren <= 1'b0;
		ram1_addr <= 0;
	end
	
	SEND1_DATA: // TX1_FSM
	begin
		ram1_addr <= ram1_addr + 1'b1;
	end
	
	default: ;
	endcase
end
//- TX1_FSM crc + ram_wren + ram_addr    ***

//+ TX_FSM data transfer
always @ (posedge e_rxc)
begin
	case (tx_state)
	IDLE:
	begin
		txen <= 1'b0;
		e_txer <= 1'b0;
		//
		txd <= 8'b0;
		ram_data_r <= 8'b0;
	end

	STOR_SW1:
	begin
		ram_data_in[3:0] <= switches;
		ram_data_in[7:4] <= switches;
	end
	
	STOR_SW2:
	begin
		case (ram_addr)
		7'd50:  ram_data_in  <= pata_version[7:0];
		7'd51:  ram_data_in  <= pata_version[15:8];
		7'd52:  ram_data_in  <= pata_pkt_num[7:0]; // set data for ram write before address set 
		7'd53:  ram_data_in  <= pata_pkt_num[15:8];
		7'd54:  ram_data_in  <= pata_flags_return[7:0]; // commend result
		7'd55:  ram_data_in  <= pata_flags_return[15:8];
		7'd56:  ram_data_in  <= ata_timeout_cnt[7:0]; // return ata_timeout_cnt for measure ATA command execution time
		7'd57:  ram_data_in  <= ata_timeout_cnt[15:8];
		7'd58:  ram_data_in  <= ata_timeout_cnt[23:16];
		7'd59:  ram_data_in  <= ata_timeout_cnt[31:24];
		7'd60:  ram_data_in  <= pata_p1[7:0];
		7'd61:  ram_data_in  <= pata_p1[15:8];
		7'd62:  ram_data_in  <= pata_p2[7:0];
		7'd63:  ram_data_in  <= pata_p2[15:8];
		7'd64:  ram_data_in  <= pata_p3[7:0];
		7'd65:  ram_data_in  <= pata_p3[15:8];
		7'd66:  ram_data_in  <= pata_p4[7:0];
		7'd67:  ram_data_in  <= pata_p4[15:8];
		7'd68:  ram_data_in  <= pata_p5[7:0];
		7'd69:  ram_data_in  <= pata_p5[15:8];
		7'd70:  ram_data_in  <= pata_p6[7:0];  // rx_data_len[7:0]
		7'd71:  ram_data_in  <= pata_p6[15:8]; // rx_data_len[15:8]
		7'd72:  ram_data_in  <= pata_p7rd; // return status reg
		7'd73:  begin
			ram_data_in <= {power_sw_trig, atacmd_done_flag, 5'd2, ata_timeout_cnt[32]}; // 5'd2 here for identify last byte of sense
		end
		7'd74:  ram_data_in  <= pata_flags_tail;
		default:;
		endcase
	end

	CALC_CRC:
	begin
		ram_data_r <= ram_data;
	end
	
	SEND_CRC1:
	begin
		ram_data_in <= ~{crc[24], crc[25], crc[26], crc[27], crc[28], crc[29], crc[30], crc[31]};
	end
	SEND_CRC2:
	begin
		ram_data_in <= ~{crc[16], crc[17], crc[18], crc[19], crc[20], crc[21], crc[22], crc[23]};
	end
	SEND_CRC3:
	begin
		ram_data_in <= ~{crc[8], crc[9], crc[10], crc[11], crc[12], crc[13], crc[14], crc[15]};
	end
	SEND_CRC4:
	begin
		ram_data_in <= ~{crc[0], crc[1], crc[2], crc[3], crc[4], crc[5], crc[6], crc[7]};
	end

	SEND_DATA:
	begin
		txen <= 1'b1;
		txd <= ram_data;
	end
	
	IDLE_CODE: 
	begin
		txen <= 1'b0;
		txd <= 8'b0;
	end
	
	default: ;
	endcase
end
//- TX_FSM data transfer

//+ TX1_FSM data transfer                ***
always @ (posedge e_rxc)
begin
	case (tx1_state)
	IDLE1: // TX1_FSM
	begin
		tx_data1 <= 1'b0;
		txen1 <= 1'b0;
		txd1 <= 8'b0;
		ram1_data_r <= 8'b0;
		tx_hdr_cp_flag <= 1'b0;
		ata2tx_txfin <= 1'b0;
	end

	STOR1_SET1: // TX1_FSM
	begin
	end

	STOR1_CP_HDR: // TX1_FSM
	begin
		case (ram1_addr)
		11'd49:  ram1_data_in  <= pata_pkt_num[7:0]; // set data for ram write before address set 
		11'd50:  ram1_data_in  <= pata_pkt_num[15:8];
		11'd51:  ram1_data_in  <= 8'hAA; // 
		11'd52:  ram1_data_in  <= 8'hBB;
		11'd53:  ram1_data_in  <= 8'hCC;
		11'd54: begin
			ram1_data_in   <= 8'hDD;
			tx_hdr_cp_flag <= 1'b1;
		end
		default:;
		endcase
	end
	
	STOR1_WAIT_W: // TX1_FSM
	begin
		tx_hdr_cp_flag <= 1'b0;
	end

	STOR1_CPY_D1: // TX1_FSM
	begin
		ram1_data_in <= pata_p0rd[7:0];
	end

	STOR1_CPY_D2: // TX1_FSM
	begin
		ram1_data_in <= pata_p0rd[15:8];
	end

	STOR1_CHKFIN: // TX1_FSM
	begin
	end

	STOR1_ZADR: // TX1_FSM
	begin
	end

	CALC1_CRC: // TX1_FSM
	begin
		ram1_data_r <= ram1_data;
	end
	
	SEND1_CRC1: // TX1_FSM
	begin
		ram1_data_in <= ~{crc1[24], crc1[25], crc1[26], crc1[27], crc1[28], crc1[29], crc1[30], crc1[31]};
	end
	SEND1_CRC2: // TX1_FSM
	begin
		ram1_data_in <= ~{crc1[16], crc1[17], crc1[18], crc1[19], crc1[20], crc1[21], crc1[22], crc1[23]};
	end
	SEND1_CRC3: // TX1_FSM
	begin
		ram1_data_in <= ~{crc1[8], crc1[9], crc1[10], crc1[11], crc1[12], crc1[13], crc1[14], crc1[15]};
	end
	SEND1_CRC4: // TX1_FSM
	begin
		ram1_data_in <= ~{crc1[0], crc1[1], crc1[2], crc1[3], crc1[4], crc1[5], crc1[6], crc1[7]};
	end

	SEND1_DATA: // TX1_FSM
	begin
		tx_data1 <= 1'b1;
		txen1 <= 1'b1;
		txd1 <= ram1_data;
	end
	
	IDLE1_CODE: // TX1_FSM
	begin
		tx_data1 <= 1'b0;
		txen1 <= 1'b0;
		txd1 <= 8'b0;
		ata2tx_txfin <= 1'b1;
	end
	
	default: ;
	endcase
end
//- TX1_FSM data transfer                ***

//+ common for TX_FSM and TX1_FSM
assign e_txen = (tx_data1) ? txen1 : txen;
assign e_txd = (tx_data1) ? txd1 : txd;

tx_buf tx_buf_inst (
   .a(ram_addr),           // input [6 : 0] a
   .d(ram_data_in),        // input [7 : 0] d
   .clk(e_rxc),            // input clk
   .we(ram_wren),          // input we
   .spo(ram_data)          // output [7 : 0] spo
);

tx_buf_sect tx_buf_sect_inst (
   .a(ram1_addr),          // input [10 : 0] a
   .d(ram1_data_in),       // input [7 : 0] d
   .clk(e_rxc),            // input clk
   .we(ram1_wren),         // input we
   .spo(ram1_data)         // output [7 : 0] spo
);

crc crc_inst
(
   .Clk                  (e_rxc),
   .Reset                (crcrst),
   .Data_in              (ram_data_r),
   .Enable               (crcen),
   .Crc                  (crc),
   .CrcNext              ()
);

crc crc1_inst
(
   .Clk                  (e_rxc),
   .Reset                (crcrst1),
   .Data_in              (ram1_data_r),
   .Enable               (crcen1),
   .Crc                  (crc1),
   .CrcNext              ()
);
//- common for TX_FSM and TX1_FSM

// +RX ////////////////////////////////////////////////////////////////////////////////

assign rx_dst_mac_w     = rx_mac[111:64];
assign rx_dst_addr_w    = rx_ip_header[31:0];
assign rx_dst_port_w    = rx_udp_header[47:32];
assign rx_ip_type       = rx_ip_header[87:80];

assign is_rxd_PRE       = (e_rxdv && e_rxd == PRESEMBLE)  ? 1'b1 : 1'b0;
assign is_prestart      = ((e_rxd == PRESTART) && e_rxdv) ? 1'b1 : 1'b0;
assign is_cur_state_n   = (rx_nxt_state != rx_state)      ? 1'b1 : 1'b0;

assign flag_rx_pre_over = (rx_cnt >= R_PRE_CNT - 16'd1)   ? 1'b1 : 1'b0;
assign flag_pre_cnt     = ((e_rxd == PRESEMBLE) && e_rxdv && rx_cnt < R_PRE_CNT - 16'd1) ? 1'b1 : 1'b0;
assign flag_mac_over    = (rx_cnt >= R_MAC_CNT - 16'd1)   ? 1'b1 : 1'b0;
assign flag_header_idle = (rx_dst_mac_w != DST_MAC)       ? 1'b1 : 1'b0;
assign flag_header_over = (rx_cnt >= R_HRD_CNT - 16'd1)   ? 1'b1 : 1'b0;
assign flag_data_idle   = (rx_dst_port_w != DST_PORT || rx_dst_addr_w != DST_ADDR || rx_ip_type != UDP_TYPE) ? 1'b1 : 1'b0;
assign flag_data_over   = (rx_data_len == 16'd0)          ? 1'b1 : 1'b0; // SY: ??? size - udp header bytes... must be zero at end

always @ (posedge e_rxc or negedge rst_n)
begin
	if (!rst_n)
		rx_state	<=	R_IDLE;
	else
		rx_state	<=	rx_nxt_state;
end

always @ (*)
begin
	rx_nxt_state = rx_state;
	//
	case (rx_state)
	R_IDLE:     // RX FSM
		if(is_rxd_PRE)
			rx_nxt_state = R_PRE;
	
	R_PRE:      // RX FSM
		if(flag_rx_pre_over)
			rx_nxt_state = R_MAC;
	
	R_MAC:      // RX FSM
		if(flag_mac_over)
			rx_nxt_state = R_HEADER;
	
	R_HEADER:   // RX FSM
		if(flag_header_idle)
			rx_nxt_state = R_IDLE;
		else if (flag_header_over)
			rx_nxt_state = R_DATA;
	
	R_DATA:     // RX FSM
		if(flag_data_idle)
			rx_nxt_state = R_IDLE;
		else if(flag_data_over)
			rx_nxt_state = R_FIHISH;
		else if(ar_flag_fill_buffer)
			rx_nxt_state = R_DATA_S;

	R_DATA_S:   // RX FSM 
		// fill write buffer with data
		if(flag_data_over)
			rx_nxt_state = R_DATA_FIN;

	R_DATA_FIN: // RX FSM
		rx_nxt_state = R_FIHISH;

	R_FIHISH: rx_nxt_state = R_IDLE;
	default: rx_nxt_state = R_IDLE;
	endcase
end

//rx_cnt
always @ (posedge e_rxc)
begin
	case (rx_state)

	R_PRE:
	begin
		if (is_cur_state_n) begin
			rx_cnt <= 16'd0;
		end else if (flag_pre_cnt)
			rx_cnt <= rx_cnt + 16'd1;
		else if (is_prestart)
			rx_cnt <= rx_cnt + 16'd1;
	end
	
	R_MAC, R_HEADER:
	begin
		if (is_cur_state_n)
			rx_cnt <= 'd0;
		else if (e_rxdv)
			rx_cnt <= rx_cnt + 16'd1;
	end
	
	default:	rx_cnt <= 16'd0;
	endcase
end

//ip_header, udp_header, mac
always @ (posedge e_rxc)
begin
	case (rx_state)
	R_IDLE:
	begin
		rx_ip_header    <= 160'd0;
		rx_udp_header   <= 64'd0;
		rx_mac          <= 112'd0;
	end
	
	R_MAC:
		if (e_rxdv) begin
			case (rx_cnt)
			16'd0:  rx_mac[111:104] <= e_rxd;
			16'd1:  rx_mac[103:96]  <= e_rxd;
			16'd2:  rx_mac[95:88]   <= e_rxd;
			16'd3:  rx_mac[87:80]   <= e_rxd;
			16'd4:  rx_mac[79:72]   <= e_rxd;
			16'd5:  rx_mac[71:64]   <= e_rxd;
			16'd6:  rx_mac[63:56]   <= e_rxd;
			16'd7:  rx_mac[55:48]   <= e_rxd;
			16'd8:  rx_mac[47:40]   <= e_rxd;
			16'd9:  rx_mac[39:32]   <= e_rxd;
			16'd10: rx_mac[31:24]   <= e_rxd;
			16'd11: rx_mac[23:16]   <= e_rxd;
			16'd12: rx_mac[15:8]    <= e_rxd;
			16'd13: rx_mac[7:0]     <= e_rxd;
			default:;
			endcase
		end
	
	R_HEADER:
		if (e_rxdv)
		begin		
			case (rx_cnt)
			16'd0:   rx_ip_header[159:152] <= e_rxd;
			16'd1:   rx_ip_header[151:144] <= e_rxd;
			16'd2:   rx_ip_header[143:136] <= e_rxd;
			16'd3:   rx_ip_header[135:128] <= e_rxd;
			16'd4:   rx_ip_header[127:120] <= e_rxd;
			16'd5:   rx_ip_header[119:112] <= e_rxd;
			16'd6:   rx_ip_header[111:104] <= e_rxd;
			16'd7:   rx_ip_header[103:96]  <= e_rxd;
			16'd8:   rx_ip_header[95:88]   <= e_rxd;
			16'd9:   rx_ip_header[87:80]   <= e_rxd;
			16'd10:  rx_ip_header[79:72]   <= e_rxd;
			16'd11:  rx_ip_header[71:64]   <= e_rxd;
			16'd12:  rx_ip_header[63:56]   <= e_rxd;
			16'd13:  rx_ip_header[55:48]   <= e_rxd;
			16'd14:  rx_ip_header[47:40]   <= e_rxd;
			16'd15:  rx_ip_header[39:32]   <= e_rxd;
			16'd16:  rx_ip_header[31:24]   <= e_rxd;
			16'd17:  rx_ip_header[23:16]   <= e_rxd;
			16'd18:  rx_ip_header[15:8]    <= e_rxd;
			16'd19:  rx_ip_header[7:0]     <= e_rxd;
			16'd20:  rx_udp_header[63:56]  <= e_rxd;
			16'd21:  rx_udp_header[55:48]  <= e_rxd;
			16'd22:  rx_udp_header[47:40]  <= e_rxd;
			16'd23:  rx_udp_header[39:32]  <= e_rxd;
			16'd24:  rx_udp_header[31:24]  <= e_rxd;
			16'd25:  rx_udp_header[23:16]  <= e_rxd;
			16'd26:  rx_udp_header[15:8]   <= e_rxd;
			16'd27:  rx_udp_header[7:0]    <= e_rxd;
			default: ;
			endcase
		end
	default: ;
	endcase
end

always @ (posedge e_rxc)
begin
	case (rx_state)
	R_IDLE:
	begin
		rx_data_cnt <= 11'd0;
		rx_atacmd_flag <= 1'b0;
		rx_atasense_flag <= 1'b0;
		ar_flag_fill_buffer <= 1'b0;
		rx_buf1_addr <= 10'd0;
		rx_buf1_we <= 1'b0;
		rx_buf1_fill_done <= 1'b0;
		rx_testbuf1_flag <= 1'b0;
	end

	R_HEADER:
	begin
		rx_data_len <= rx_udp_header[31:16];
	end

	R_DATA:
	begin
		if (e_rxdv) begin
			rx_data_len  <= rx_data_len - 16'd1;
			rx_data_cnt  <= rx_data_cnt + 11'd1;
			//
			case(rx_data_cnt) 
			11'd0:  pata_pkt_num_dummy[7:0]  <= e_rxd;
			11'd1:  pata_pkt_num_dummy[15:8] <= e_rxd;
			11'd2:  begin
				pata_flags_dummy[7:0]  <= e_rxd;
				pata_flags[7:0]  <= e_rxd;
				if(e_rxd[0] == 1'b1) begin   // ask sense command detected
					rx_data_len <= 16'd0;     // stop copy packet content, makes flag_data_over = 1
					rx_atasense_flag <= 1'b1; // send sense once more
					if(e_rxd[1] == 1'b1) begin   // power off
						power_sw_trig <= 1'b0;
					end else begin
						if(e_rxd[2] == 1'b1) begin   // power on
							power_sw_trig <= 1'b1;
						end
					end
				end else if(e_rxd[6] == 1'b1) begin
					ar_flag_fill_buffer <= 1'b1;
				end
				// Если стоит флаг - запись в буфер, то последующие байты заголовка принимаются в другие регистры,
				// а по приему заголовка вся шаговая машина должна перейти на другой шаг, где заполнить нужный буфер данными.
				// После чего поставить флаг, что буфер с данными корректен.
			end
			11'd3:  pata_flags_dummy[15:8] <= e_rxd;
			11'd4:  pata_timeout_before[7:0]  <= e_rxd;
			11'd5:  pata_timeout_before[15:8] <= e_rxd;
			11'd6:  pata_timeout_after[7:0]  <= e_rxd;
			11'd7:  pata_timeout_after[15:8] <= e_rxd;
			11'd8:  pata_p1[7:0]  <= e_rxd;
			11'd9:  pata_p1[15:8] <= e_rxd;
			11'd10: pata_p2[7:0]  <= e_rxd;
			11'd11: pata_p2[15:8] <= e_rxd;
			11'd12: pata_p3[7:0]  <= e_rxd;
			11'd13: pata_p3[15:8] <= e_rxd;
			11'd14: pata_p4[7:0]  <= e_rxd;
			11'd15: pata_p4[15:8] <= e_rxd;
			11'd16: pata_p5[7:0]  <= e_rxd;
			11'd17: pata_p5[15:8] <= e_rxd;
			11'd18: pata_p6[7:0]  <= e_rxd;
			11'd19: pata_p6[15:8] <= e_rxd;
			11'd20: pata_p7[7:0]  <= e_rxd;
			11'd21: begin // 22 bytes income
				pata_p7[15:8] <= e_rxd; // emit exec command, check CRC ?
				rx_atacmd_flag <= 1'b1; // exec (AR_SET1 of ATA_FSM) and (STOR_SW1 of TX_FSM)
				pata_pkt_num <= pata_pkt_num_dummy; // for ask sense command
				pata_flags <= pata_flags_dummy;
				rx_data_len <= 16'd0; // stop copy packet content
				end
			endcase
		end
	end
	
	R_DATA_S: // fill write buffer with data
	begin
		if (e_rxdv) begin
			rx_data_len  <= rx_data_len - 16'd1;
			rx_data_cnt  <= rx_data_cnt + 11'd1;
			// fill variables
			case(rx_data_cnt) 
			11'd3:  pata_flags[15:8]  <= e_rxd;       // high part of pata_flags
			11'd4:  pata_cmd_pkt_num[7:0]  <= e_rxd;  // номер команды ??? compare ???
			11'd5:  pata_cmd_pkt_num[15:8] <= e_rxd;
			11'd6:  pata_buf_fill_num  <= e_rxd;      // номер буфера для заполнения
			11'd7:  pata_cmd_sect_num[7:0] <= e_rxd;  // номер сектора в команде
			11'd8:  begin 
				pata_cmd_sect_num[15:8] <= e_rxd;
				rx_buf1_we <= 1'b1; // if(e_rxdv) ??? disable rx_buf1_we if not e_rxdv
			end
			11'd520: begin // 22 bytes income
				rx_data_len <= 16'd0; // stop copy packet content
				rx_buf1_we <= 1'b0;
				end
			default: begin
				rx_buf1_addr <= rx_buf1_addr + 10'd1; // fill buffer RAM
				rx_buf1_we <= 1'b1; // if(e_rxdv) ??? disable rx_buf1_we if not e_rxdv
			end
			endcase
		end else begin
			rx_buf1_we <= 1'b0; // if(e_rxdv) ??? disable rx_buf1_we if not e_rxdv
		end
	end

	R_DATA_FIN:
	begin
		rx_buf1_we <= 1'b0;
		// флаг заполнения буфера
		rx_buf1_fill_done <= 1'b1; // продумать сброс этого флага - сброс сразу, а всю кухню делает шаговая машина state_rxbuf1
		//
		if(ar_flag_do_read_PIO == 1'b1) begin // pata_flags[3]
			rx_testbuf1_flag <= 1'b1;
		end
	end

	default:	;
	endcase
end

localparam [1:0]
	rxbuf1_fill_wait = 0,
	rxbuf1_send_wait = 1;

reg[1:0]       state_rxbuf1, state_rxbuf1_next;

always @ (posedge e_rxc or negedge rst_n) begin
	if(!rst_n) begin
		state_rxbuf1 <= rxbuf1_fill_wait;
	end else begin
		state_rxbuf1 <= state_rxbuf1_next;
	end
end

always @(state_rxbuf1, rx_buf1_fill_done, rx_buf1_fill_reset) begin
	state_rxbuf1_next = state_rxbuf1; // default state_next
	case (state_rxbuf1)
		rxbuf1_fill_wait: begin
			if(rx_buf1_fill_done == 1'b1) begin
				state_rxbuf1_next = rxbuf1_send_wait;
			end
		end
		rxbuf1_send_wait: begin
			if(rx_buf1_fill_reset == 1'b1) begin
				state_rxbuf1_next = rxbuf1_fill_wait;
			end
		end
	endcase
end

always @(state_rxbuf1) begin
	// default
	rx_buf1_filled = 1'b0;
	case (state_rxbuf1)
		rxbuf1_fill_wait: begin
			rx_buf1_filled = 1'b0;
		end
		rxbuf1_send_wait: begin
			rx_buf1_filled = 1'b1;
		end
	endcase
end

rxbufsect rx_buf_sect_inst (
   .a(rx_buf1_addr),       // input [9 : 0] a
   .d(e_rxd),              // input [7 : 0] d
   .dpra(pata_buf1_addr),  // input [9 : 0] dpra
   .clk(e_rxc),            // input clk
   .we(rx_buf1_we),        // input we
   .dpo(pata_buf1_data)    // output [7 : 0] dpo
);


// -RX ////////////////////////////////////////////////////////////////////////////////

// +PATA host //////////////////////////////////////////////////////////////////////////

//+ ATA_FSM state definition
localparam
	AR_IDLE         = 6'b000000, 
	AR_SET1         = 6'b000001,
	AR_WAIT1CYC     = 6'b000010, // read +7 before cmd wait cycle
	AR_WAIT1        = 6'b000011,
	AR_TIMEOUT1     = 6'b000100,
	AR_WRITE_CMD1   = 6'b000101, // load data
	AR_WRITE_CMD2   = 6'b000110, // 1 clk wait for set data before strobe
	AR_WRITE_CMD3   = 6'b000111, // make IOW strobe cycle
	AR_SET2         = 6'b001000,
	AR_WAIT2CYC     = 6'b001001, // read +7 after cmd wait cycle
	AR_WAIT2        = 6'b001010,
	AR_TIMEOUT2     = 6'b001011,
	AR_CMD_SENT     = 6'b001100, // command send and drive become ready
	AR_PIO_RD1      = 6'b001101, // like AR_SET1 or AR_SET2
	AR_PIO_RD2CYC   = 6'b001110, // read +7
	AR_PIO_RD2WT    = 6'b001111,
	AR_TIMEOUT3     = 6'b010000, // DRQ timeout
	AR_PIO_RD3SET   = 6'b010001, // run UDP send
	AR_PIO_RD3CYC   = 6'b010010, // +0 read cycle
	AR_PIO_RD3CHK   = 6'b010011, // check for counter and wait for UDP packet send
	AR_PIO_RD3WT    = 6'b010100, // emit signal to TX1_FSM
	AR_PIO_WR1      = 6'b010101,
	AR_PIO_WR2CYC   = 6'b010110,
	AR_PIO_WR2WT    = 6'b010111, // check DRQ and write buffer fill finish step on PIO write process
	AR_PIO_WR3SET   = 6'b011000,
	AR_PIO_WR3CYC   = 6'b011001, // +0 write cycle
	AR_PIO_WR3CHK   = 6'b011010,
	AR_PIO_WR3FIN   = 6'b011011,
	AR_P7RD_SET     = 6'b011100, // read +7 one time begin
	AR_P7RD_CYC     = 6'b011101, // read +7 one time cycle
	AR_TESTBUF1_SET = 6'b011110, // copy write buffer 1 to read buffer and send via ethernet
	AR_TESTBUF1_CYC = 6'b011111,
	AR_TESTBUF1_CHK = 6'b100000,
	AR_TESTBUF1_FIN = 6'b100001,
	AR_FIHISH       = 6'b111111;
//- ATA_FSM state definition

//??? !!! read cycle sectors cnt

reg  [5:0]      ata_ready_state;       /*synthesis preserve*/
reg  [5:0]      ata_ready_next_state;  /*synthesis preserve*/
wire [5:0]      ata_read_wait_cnt;     // 6 bit cnt
reg             ata_read1_cyc_finish;  // +7 read cycle finish flag WAIT BEFORE
reg             ata_read2_cyc_finish;  // +7 read cycle finish flag WAIT AFTER
reg             ata_rd2_cyc_finish;    // +7 read cycle finish flag WAIT DRQ
reg             ata_data_oe;
reg  [15:0]     ata_data_out;
reg  [32:0]     ata_timeout_cnt; // 33 bit counter, full scale: (2^33) / 125M = 68.71s
wire [7:0]      ar_wait_ready_mask;
wire            ar_flag_skip_cmd;
wire            ar_flag_skip_after;
reg  [3:0]      ata_reg_address;
reg  [5:0]      ata_write_reg_cnt; // clk cycles per DIOW pulse 
reg             ata_iow_cyc_finish;
reg             ata_udp_data_tx_in_progress; // udp_send_confirm 1 at start, 0 at recive confirm pkt - make confirm pkt
reg  [4:0]      ata_read_p0_cnt;
reg  [8:0]      ata_read_words_cnt;
reg             ata_wr2_cyc_finish;    // +7 read cycle finish flag WAIT DRQ
reg             ata_wr3_cyc_finish;    // +0 write cycle finish flag
reg  [4:0]      ata_write_p0_cnt;
reg  [4:0]      ata_read_p7_cnt;
reg             ata_read_p7_cyc_finish;  // +7 read cycle finish flag IDLE, error reg +1 read?
reg  [26:0]     ata_idle_cnt; // 27 bit counter, full scale: (2^26) / 125M = 0.5368s

assign pata_dd = (ata_data_oe == 1'b1) ? ata_data_out : 16'bz;

assign ar_wait_ready_mask = pata_p7[15:8];
assign flag_ata_ready = ( (pata_p7rd & {1'b1, ar_wait_ready_mask[6:0]}) == ar_wait_ready_mask ) ? 1'b1 : 1'b0; // check for example (pata_p7rd & 0x50 == 0x50)
assign flag_timeout_before = (ata_timeout_cnt[32:17] >= pata_timeout_before) ? 1'b1 : 1'b0;
assign flag_timeout_after = (ata_timeout_cnt[32:17] >= pata_timeout_after) ? 1'b1 : 1'b0;
assign flag_ata_drq_ready = ( ((pata_p7rd & {1'b1, ar_wait_ready_mask[6:0]}) == ar_wait_ready_mask) && ((pata_p7rd & 8'h08) == 8'h08) ) ? 1'b1 : 1'b0; // example 0x58

assign ar_flag_skip_cmd = pata_flags[1];
assign ar_flag_skip_after = pata_flags[2];
assign ar_flag_do_read_PIO = pata_flags[3];
assign ar_flag_do_write_PIO = pata_flags[4];
assign ar_flag_ext_cmd = pata_flags[5]; // !!! not implemented yet

assign ata_read_wait_cnt = ata_timeout_cnt[4:0];

assign ata_write_cyc_finish = ata_reg_address[3];

assign pata_flags_tail = { 7'b1000000, rx_buf1_filled};

always @ (posedge e_rxc or negedge rst_n)
begin
	if (!rst_n)
		ata_ready_state <= AR_IDLE;
	else
		ata_ready_state <= ata_ready_next_state;
end

//+ ATA_FSM state selection
always @ (*) 
begin
	ata_ready_next_state = ata_ready_state;
	//
	case (ata_ready_state)
	AR_IDLE:
		if(rx_atacmd_flag) begin
			ata_ready_next_state = AR_SET1;
		end else if(rx_testbuf1_flag) begin
			ata_ready_next_state = AR_TESTBUF1_SET;
		end else begin
			if(ata_idle_cnt[26] == 1'b1)
				ata_ready_next_state = AR_P7RD_SET;
		end
	
	AR_SET1:
		if(ar_flag_skip_cmd) begin
			if( (ar_flag_do_read_PIO == 1'b1) && (ar_flag_do_write_PIO == 1'b0) ) begin
				ata_ready_next_state = AR_PIO_RD3SET;
			end else begin
				if( (ar_flag_do_read_PIO == 1'b0) && (ar_flag_do_write_PIO == 1'b1) ) begin
					ata_ready_next_state = AR_PIO_WR3SET;
				end
			end
		end else begin
			ata_ready_next_state = AR_WAIT1CYC;
		end
	
	AR_WAIT1CYC: // WAIT BEFORE
		if(flag_timeout_before) begin // compare timeout counter here 31:16
			ata_ready_next_state = AR_TIMEOUT1;
		end else begin
			if(ata_read1_cyc_finish)
				ata_ready_next_state = AR_WAIT1;
		end
	
	AR_WAIT1:
		if(flag_ata_ready) begin // if disk not ready then try once more
			ata_ready_next_state = AR_WRITE_CMD1;
		end else begin
			ata_ready_next_state = AR_WAIT1CYC;
		end
	
	AR_TIMEOUT1:
		ata_ready_next_state = AR_FIHISH;

	AR_WRITE_CMD1:
		ata_ready_next_state = AR_WRITE_CMD2;

	AR_WRITE_CMD2:
		ata_ready_next_state = AR_WRITE_CMD3;

	AR_WRITE_CMD3:
		if(ata_write_cyc_finish) begin
			if(ar_flag_skip_after)
				ata_ready_next_state = AR_FIHISH;
			else
				ata_ready_next_state = AR_SET2;
		end else begin
			if(ata_iow_cyc_finish)
				ata_ready_next_state = AR_WRITE_CMD1;
		end

	AR_SET2:
		ata_ready_next_state = AR_WAIT2CYC;

	AR_WAIT2CYC: // WAIT AFTER
		if(ata_read2_cyc_finish)
			ata_ready_next_state = AR_WAIT2;
	
	AR_WAIT2:
		if(flag_ata_ready) begin // if disk not ready then try once more
			if( (ar_flag_do_read_PIO == 1'b1) && (ar_flag_do_write_PIO == 1'b0) ) begin
				ata_ready_next_state = AR_PIO_RD1;
			end else begin
				if( (ar_flag_do_read_PIO == 1'b0) && (ar_flag_do_write_PIO == 1'b1) ) begin
					ata_ready_next_state = AR_PIO_WR1;
				end else begin
					ata_ready_next_state = AR_CMD_SENT; // drive become ready after non data command
				end
			end
		end else begin
			if(flag_timeout_after) begin // compare timeout counter here
				ata_ready_next_state = AR_TIMEOUT2;
			end else begin
				ata_ready_next_state = AR_WAIT2CYC;
			end
		end

	AR_TIMEOUT2:
		ata_ready_next_state = AR_FIHISH;

	AR_CMD_SENT: // non data command success exit
		ata_ready_next_state = AR_FIHISH;

	AR_PIO_RD1:
		ata_ready_next_state = AR_PIO_RD2CYC;

	AR_PIO_RD2CYC: // WAIT DRQ
		if(ata_rd2_cyc_finish)
			ata_ready_next_state = AR_PIO_RD2WT;
	
	AR_PIO_RD2WT: // check DRQ
		if(flag_ata_drq_ready) begin
			ata_ready_next_state = AR_PIO_RD3SET; // continue reading data
		end else begin
			if(flag_timeout_after) begin // compare timeout counter here
				ata_ready_next_state = AR_TIMEOUT3;
			end else begin
				ata_ready_next_state = AR_PIO_RD2CYC;
			end
		end

	AR_TIMEOUT3:
		ata_ready_next_state = AR_FIHISH;

	AR_PIO_RD3SET: // # begin of HDD data read cycle, set ata address = 0
		ata_ready_next_state = AR_PIO_RD3CYC;

	AR_PIO_RD3CYC: // +0 read cycle
		if(ata_read3_cyc_finish)
			ata_ready_next_state = AR_PIO_RD3CHK;
		else
			ata_ready_next_state = AR_PIO_RD3CYC;

	AR_PIO_RD3CHK: // check for words of sector counter and wait for UDP packet send
		if(ata_read_words_cnt == 9'h100)
			ata_ready_next_state = AR_PIO_RD3WT; // go to UDP packet send
		else
			ata_ready_next_state = AR_PIO_RD3CYC;

	AR_PIO_RD3WT:
		if(ata2tx_txfin) // wait for TX1_FSM finish transmit UDP packet
			ata_ready_next_state = AR_FIHISH;

	AR_PIO_WR1:
		ata_ready_next_state = AR_PIO_WR2CYC;
		
	AR_PIO_WR2CYC: // WAIT DRQ
		if(ata_wr2_cyc_finish)
			ata_ready_next_state = AR_PIO_WR2WT;

	AR_PIO_WR2WT: // check DRQ and write buffer fill finish
		if( (flag_ata_drq_ready == 1'b1) && (rx_buf1_filled == 1'b1) ) begin // DRQ and write buffer will check (rx_buf1_filled == 1)
			ata_ready_next_state = AR_PIO_WR3SET; // continue reading data
		end else begin
			if(flag_timeout_after) begin // compare timeout counter here
				ata_ready_next_state = AR_TIMEOUT3;
			end else begin
				ata_ready_next_state = AR_PIO_WR2CYC;
			end
		end

	AR_PIO_WR3SET:
		ata_ready_next_state = AR_PIO_WR3CYC;

	AR_PIO_WR3CYC: // +0 write cycle
		if(ata_wr3_cyc_finish)
			ata_ready_next_state = AR_PIO_WR3CHK;

	AR_PIO_WR3CHK: // check for words of sector counter
		if(pata_buf1_addr == 10'h200)
			ata_ready_next_state = AR_PIO_WR3FIN; // cycle exit
		else
			ata_ready_next_state = AR_PIO_WR3CYC;

	AR_PIO_WR3FIN:
		ata_ready_next_state = AR_FIHISH;

	AR_P7RD_SET:
		ata_ready_next_state = AR_P7RD_CYC;

	AR_P7RD_CYC:
		if(ata_read_p7_cyc_finish)
			ata_ready_next_state = AR_IDLE;

	AR_TESTBUF1_SET: // test write buffer by copy to read buffer
		if(ata_write_reg_cnt == 6'd8) // wait for copy header
			ata_ready_next_state = AR_TESTBUF1_CYC;

	AR_TESTBUF1_CYC:
		if(ata_wr3_cyc_finish)
			ata_ready_next_state = AR_TESTBUF1_CHK;

	AR_TESTBUF1_CHK:
		if(pata_buf1_addr == 10'h200)
			ata_ready_next_state = AR_TESTBUF1_FIN; // cycle exit
		else
			ata_ready_next_state = AR_TESTBUF1_CYC;

	AR_TESTBUF1_FIN:
		if(ata2tx_txfin) // wait for TX1_FSM finish transmit UDP packet
			ata_ready_next_state = AR_FIHISH;

	AR_FIHISH: ata_ready_next_state = AR_IDLE;
	default: ata_ready_next_state = AR_IDLE;
	endcase
end
//- ATA_FSM state selection

//+ ATA_FSM signal control
always @ (posedge e_rxc)
begin
	case (ata_ready_state)

	AR_IDLE:
	begin
		pata_RESETn <= 1'b1;
		pata_da <= 3'b000;
		ata_reg_address <= 4'b0001;
		pata_CSn <= 2'b11;
		ata_read1_cyc_finish <= 1'b0;
		ata_read2_cyc_finish <= 1'b0;
		ata_read3_cyc_finish <= 1'b0;
		pata_DIOWn <= 1'b1;
		pata_DIORn <= 1'b1;
		ata_data_oe <= 1'b0;
		ata_data_out <= 16'd0;
//		ata_timeout_cnt <= 33'd0; // timeout counter not zero at AR_IDLE
		ata_write_reg_cnt <= 6'd0;
		ata_iow_cyc_finish <= 1'b0;
		atacmd_done_flag <= 1'b0;
		ata_rd2_cyc_finish <= 1'b0;
		ata_udp_data_tx_in_progress <= 1'b0;
		ata_read_words_cnt <= 9'd0;
		tx2ata_sectfin <= 1'b0;
		pata_buf1_addr <= 10'd0; // write buffer address
		ata_wr3_cyc_finish <= 1'b0;
		rx_buf1_fill_reset <= 1'b0;
		ata_read_p7_cyc_finish <= 1'b0;
		ata_idle_cnt <= ata_idle_cnt + 27'd1; // increment idle counter
	end

	AR_SET1:
	begin // set address +7 and CS# 10
		pata_da <= 3'b111;
		pata_CSn <= 2'b10;
		ata_timeout_cnt <= 33'd0; // start timeout counter here
		pata_flags_return <= 16'd0; // zero all return flags at command cycle begin
	end

	AR_WAIT1CYC: // WAIT BEFORE
	begin
		// тут можно обойтись одним большим счетчиком этим:
		ata_timeout_cnt <= ata_timeout_cnt + 33'd1;
		case(ata_read_wait_cnt)
		5'd0: begin
			pata_DIORn <= 1'b0; // activate read
		end
		5'd14: begin // fix read data
			 pata_p7rd <= pata_dd[7:0];
		end
		5'd15: begin
			pata_DIORn <= 1'b1; // stop read
			ata_read1_cyc_finish <= 1'b1;
		end
		default: ;
		endcase
	end

	AR_WAIT1:
	begin
		ata_read1_cyc_finish <= 1'b0;
	end

	AR_TIMEOUT1:
	begin
		atacmd_done_flag <= 1'b1;
		pata_flags_return[0] <= 1'b1;
	end

	AR_WRITE_CMD1:
	begin
		// set data value to ata data bus
		ata_data_oe <= 1'b1;
		pata_da <= ata_reg_address[2:0];
		ata_iow_cyc_finish <= 1'b0;
		//
		case(ata_reg_address)
		4'd1: ata_data_out[7:0] <= pata_p1[7:0];
		4'd2: ata_data_out[7:0] <= pata_p2[7:0];
		4'd3: ata_data_out[7:0] <= pata_p3[7:0];
		4'd4: ata_data_out[7:0] <= pata_p4[7:0];
		4'd5: ata_data_out[7:0] <= pata_p5[7:0];
		4'd6: ata_data_out[7:0] <= pata_p6[7:0];
		4'd7: ata_data_out[7:0] <= pata_p7[7:0];
		default: ;
		endcase
	end

	AR_WRITE_CMD2:
	begin
		ata_write_reg_cnt <= 6'd0;
	end

	AR_WRITE_CMD3:
	begin
		ata_write_reg_cnt <= ata_write_reg_cnt + 6'd1; // inc counter
		//
		case(ata_write_reg_cnt)
		6'd0: begin
			pata_DIOWn <= 1'b0; // activate read
		end
		6'd14: begin // fix read data
			pata_DIOWn <= 1'b1; // stop read
		end
		6'd15: begin
			ata_iow_cyc_finish <= 1'b1;
			ata_data_oe <= 1'b0;
			ata_reg_address <= ata_reg_address + 4'd1;
		end
		default: ;
		endcase
	end
	
	AR_SET2:
	begin // set address +7 and CS# 10
		pata_da <= 3'b111;
		pata_CSn <= 2'b10;
		ata_timeout_cnt <= 33'd0; // start timeout counter here
	end

	AR_WAIT2CYC: // WAIT AFTER
	begin
		// тут можно обойтись одним большим счетчиком
		// этим:
		ata_timeout_cnt <= ata_timeout_cnt + 33'd1;
		case(ata_read_wait_cnt)
		5'd0: begin
			pata_DIORn <= 1'b0; // activate read
		end
		5'd14: begin // fix read data
			pata_p7rd <= pata_dd[7:0];
		end
		5'd15: begin
			pata_DIORn <= 1'b1; // stop read
			ata_read2_cyc_finish <= 1'b1;
		end
		default: ;
		endcase
	end

	AR_WAIT2:
	begin
		ata_read2_cyc_finish <= 1'b0;
	end

	AR_TIMEOUT2:
	begin
		atacmd_done_flag <= 1'b1;
		pata_flags_return[1] <= 1'b1;
	end

	AR_CMD_SENT:
	begin
		atacmd_done_flag <= 1'b1; // send auto sense on HDD ready after command
		pata_flags_return[2] <= 1'b1;
	end

	AR_PIO_RD1:
	begin
		pata_da <= 3'b111;
		pata_CSn <= 2'b10;
		ata_timeout_cnt <= 33'd0; // start timeout counter here
		ata_udp_data_tx_in_progress <= 1'b1; // activate transfer
		pata_flags_return[4] <= 1'b1;
	end

	AR_PIO_RD2CYC: // WAIT DRQ
	begin
		ata_timeout_cnt <= ata_timeout_cnt + 33'd1;
		case(ata_read_wait_cnt)
		5'd0: begin
			pata_DIORn <= 1'b0; // activate read
		end
		5'd14: begin // fix read data
			pata_p7rd <= pata_dd[7:0];
		end
		5'd15: begin
			pata_DIORn <= 1'b1; // stop read
			ata_rd2_cyc_finish <= 1'b1;
		end
		default: ;
		endcase
	end
	
	AR_PIO_RD2WT: // check DRQ
	begin
		ata_rd2_cyc_finish <= 1'b0;
	end

	AR_TIMEOUT3:
	begin
		atacmd_done_flag <= 1'b1;
		pata_flags_return[3] <= 1'b1;
	end

	AR_PIO_RD3SET: // # begin of HDD data read cycle
	begin
		pata_da <= 3'b000;
		pata_CSn <= 2'b10;
		ata_read_p0_cnt <= 5'd0;
		ata_read_words_cnt <= 9'd0;
		pata_flags_return[5] <= 1'b1;
		ata_read3_cyc_finish <= 1'b0;
	end

	AR_PIO_RD3CYC: // +0 read cycle
	begin
		ata_read_p0_cnt <= ata_read_p0_cnt + 5'd1;
		case(ata_read_p0_cnt)
		5'd2: begin
			pata_DIORn <= 1'b0; // activate read
		end
		5'd8: begin // fix read data
			pata_p0rd <= pata_dd;
		end
		5'd9: begin
			pata_DIORn <= 1'b1; // stop read
			ata_read3_cyc_finish <= 1'b1;
			ata_read_words_cnt <= ata_read_words_cnt + 9'd1;
		end
		default: ;
		endcase
	end

	AR_PIO_RD3CHK:
	begin
		ata_read_p0_cnt <= 5'd0;
		ata_read3_cyc_finish <= 1'b0;
	end

	AR_PIO_RD3WT:
	begin
		tx2ata_sectfin <= 1'b1;
		pata_flags_return[6] <= 1'b1;
	end

	AR_PIO_WR1: // write setup
	begin
		pata_da <= 3'b111;
		pata_CSn <= 2'b10;
		ata_timeout_cnt <= 33'd0; // start timeout counter here
		ata_wr2_cyc_finish <= 1'b0;
	end

	AR_PIO_WR2CYC: // WAIT DRQ
	begin
		ata_timeout_cnt <= ata_timeout_cnt + 33'd1;
		case(ata_read_wait_cnt)
		5'd0: begin
			pata_DIORn <= 1'b0; // activate read
		end
		5'd14: begin // fix read data
			pata_p7rd <= pata_dd[7:0];
		end
		5'd15: begin
			pata_DIORn <= 1'b1; // stop read
			ata_wr2_cyc_finish <= 1'b1;
		end
		default: ;
		endcase
	end

	AR_PIO_WR2WT: // check DRQ
	begin
		ata_wr2_cyc_finish <= 1'b0;
	end

	AR_PIO_WR3SET: // # begin of HDD data write cycle
	begin
		pata_da <= 3'b000;
		pata_CSn <= 2'b10;
		pata_flags_return[5] <= 1'b1;
		ata_data_oe <= 1'b1; // switch PATA data direction
		ata_wr3_cyc_finish <= 1'b0;
		ata_write_p0_cnt <= 5'd0;
		pata_buf1_addr <= 10'd0; // write buffer address
	end

	AR_PIO_WR3CYC: // +0 write cycle
	begin
		ata_write_p0_cnt <= ata_write_p0_cnt + 5'd1;
		case(ata_write_p0_cnt)
		5'd0: begin // load data from memory
			ata_data_out[7:0] <= pata_buf1_data;
			pata_buf1_addr <= pata_buf1_addr + 10'd1; // write buffer address increment
		end
		5'd1: begin // load data from memory next address
			ata_data_out[15:8] <= pata_buf1_data;
			pata_buf1_addr <= pata_buf1_addr + 10'd1; // write buffer address increment
		end
		5'd2: begin
			pata_DIOWn <= 1'b0; // activate write
		end
		5'd9: begin
			pata_DIOWn <= 1'b1; // stop write
			ata_wr3_cyc_finish <= 1'b1;
		end
		default: ;
		endcase
	end

	AR_PIO_WR3CHK: // check for words of sector counter
	begin
		ata_write_p0_cnt <= 5'd0;
		ata_wr3_cyc_finish <= 1'b0;
	end

	AR_PIO_WR3FIN:
	begin
		ata_data_oe <= 1'b0;
		rx_buf1_fill_reset <= 1'b1; // сброс идет через процесс state_rxbuf1
		pata_flags_return[7] <= 1'b1;
		atacmd_done_flag <= 1'b1; // auto send sense on write sector complete
	end

	AR_P7RD_SET:
	begin // set address +7 and CS# 10
		pata_da <= 3'b111;
		pata_CSn <= 2'b10;
		ata_read_p7_cnt <= 5'd0;
		ata_idle_cnt <= 27'd0; // reset IDLE counter
	end

	AR_P7RD_CYC:
	begin
		ata_read_p7_cnt <= ata_read_p7_cnt + 5'd1;
		case(ata_read_p7_cnt)
		5'd0: begin
			pata_DIORn <= 1'b0; // activate read
		end
		5'd14: begin // fix read data
			 pata_p7rd <= pata_dd[7:0];
		end
		5'd15: begin
			pata_DIORn <= 1'b1; // stop read
			ata_read_p7_cyc_finish <= 1'b1;
		end
		default: ;
		endcase
	end

	AR_TESTBUF1_SET: // test write buffer by copy to read buffer
	begin
		ata_write_p0_cnt <= 5'd0;
		ata_read3_cyc_finish <= 1'b0;
		ata_wr3_cyc_finish <= 1'b0;
		ata_udp_data_tx_in_progress <= 1'b1; // activate transfer, reset by IDLE
		ata_write_reg_cnt <= ata_write_reg_cnt + 6'd1; // inc counter
	end

	AR_TESTBUF1_CYC: // copy write memory to read memory cycle
	begin
		ata_write_p0_cnt <= ata_write_p0_cnt + 5'd1;
		case(ata_write_p0_cnt)
		5'd0: begin // load data from memory
			ata_data_out[7:0] <= pata_buf1_data;
			pata_buf1_addr <= pata_buf1_addr + 10'd1; // write buffer address increment
		end
		5'd1: begin // load data from memory next address
			ata_data_out[15:8] <= pata_buf1_data;
			pata_buf1_addr <= pata_buf1_addr + 10'd1; // write buffer address increment
		end
		5'd2: begin
			pata_p0rd <= ata_data_out;
			ata_read3_cyc_finish <= 1'b1; // perform writing
		end
		5'd4: ata_read3_cyc_finish <= 1'b0;
		5'd6: begin // 3 to 6 is wait for TX1 FSM finish store pata_p0rd to TX1 memory
			ata_wr3_cyc_finish <= 1'b1;
		end
		default: ;
		endcase
	end
	
	AR_TESTBUF1_CHK: // check for sector copy
	begin
		ata_write_p0_cnt <= 5'd0;
		ata_wr3_cyc_finish <= 1'b0;
	end

	AR_TESTBUF1_FIN:
	begin
		tx2ata_sectfin <= 1'b1; // to stop copy loop in TX1 FSM; holding it to 1 till packet TX finished
	end

	AR_FIHISH:
	begin
		atacmd_done_flag <= 1'b0;
		pata_flags_return[6] <= 1'b0;
	end
	
	default:	;
	endcase
end
//- ATA_FSM signal control

// -PATA host //////////////////////////////////////////////////////////////////////////

reg [31:0] count;
reg [31:0] max_count;

always @(posedge e_rxc, negedge rst_n) begin
	if (!rst_n) begin
	   max_count = 0;
	end else begin
      max_count = 54999999;
//      max_count = 150;
	end
end

always @(posedge e_rxc, negedge rst_n) begin
	if(!rst_n) begin
		count <= 0;
		pkt_cnt <= 0;
		power_sw <= 2'd0;
	end
	else begin
		if( restart ) begin
			count <= 0;
			pkt_cnt <= pkt_cnt + 1;
		end
		else begin
			count <= count + 1;
			power_sw[0] <= power_sw_trig;
			power_sw[1] <= power_sw_trig;
		end
	end
end

always @(posedge e_rxc, negedge rst_n) begin
	if(!rst_n) begin
		restart <= 1'b0;
	end
	else begin
		if( count == max_count ) begin
			restart <= 1'b1;
		end
		else begin
			restart <= 1'b0;
		end
	end
end

endmodule
