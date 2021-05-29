module EEE_IMGPROC(
    // global clock & reset
    clk,
    reset_n,

    // mm slave
    s_chipselect,
    s_read,
    s_write,
    s_readdata,
    s_writedata,
    s_address,

    // stream sink
    sink_data,
    sink_valid,
    sink_ready,
    sink_sop,
    sink_eop,

    // streaming source
    source_data,
    source_valid,
    source_ready,
    source_sop,
    source_eop,

    // conduit
    mode,
	 erosion_mode,
	 dilation_mode

);

// global clock & reset
input   clk;
input   reset_n;

// mm slave
input   s_chipselect;
input   s_read;
input   s_write;
output  reg	[31:0]  s_readdata;
input       [31:0]	s_writedata;
input       [2:0]   s_address;

// streaming sink
input   [23:0]      sink_data;
input   sink_valid;
output  sink_ready;
input   sink_sop;
input   sink_eop;

// streaming source
output  [23:0]  source_data;
output  source_valid;
input   source_ready;
output  source_sop;
output  source_eop;

// conduit export
input   mode;
input   erosion_mode;
input   dilation_mode;

////////////////////////////////////////////////////////////////////////

parameter IMAGE_W = 11'd640;
parameter IMAGE_H = 11'd480;
parameter MESSAGE_BUF_MAX = 256;
parameter MSG_INTERVAL = 30;
parameter BB_COL_DEFAULT = 24'h00ff00;

wire [7:0]   red_inter_out,green_inter_out,blue_inter_out;
wire [7:0]   red_out, green_out, blue_out;
wire [7:0]   grey;
wire         red_detect, green_detect, blue_detect, grey_detect, yellow_detect;
wire         in_valid, out_ready, sop_in, eop_in, valid_rgbhsv;
wire [8:0]   hue;
wire [7:0]   saturation, value_b;
wire [21:0]  pixel_addr, pixel_addr_interm, pixel_addr_obstacle;
wire [10:0]  prev_x, prev_y;
reg  [19:0]  bright_pix_count;

////////////////////////////////////////////////////////////////////////
//Streaming registers to buffer video signal
STREAM_REG #(.DATA_WIDTH(26)) in_reg (
    .clk(clk),
    .rst_n(reset_n),
    .ready_out(sink_ready),
    .valid_out(in_valid),
    .data_out({red_out,green_out,blue_out,sop_in,eop_in}),
    .ready_in(out_ready),
    .valid_in(sink_valid),
    .data_in({sink_data,sink_sop,sink_eop})
);

///////////////////////////////////////////////////////////////////////
//RGB to HSV

rgb_to_hsv rgb_hsv(
    .clk(clk),
    .rst_n(reset_n),
    .pixel_addr(pixel_addr),
    .rgb_r(red_out),
    .rgb_g(green_out),
    .rgb_b(blue_out),	
    .hsv_h(hue),//  0 - 360
    .hsv_s(saturation),// 0- 255
    .hsv_v(value_b), // 0- 255
    .sop(sop_in),
    .valid_in(in_valid),
    .valid_out(valid_rgbhsv),
    .pixel_addr_out(pixel_addr_interm)
);

assign pixel_addr = {x ,y};
assign prev_x = pixel_addr_interm[21:11];
assign prev_y = pixel_addr_interm[10:0];

///////////////////////////////////////////////////////////////////////
// Color Threshold

colour_threshold c_th (
    .clk(clk),
    .rst_n(reset_n),
    .hue(hue),//  0 - 360
    .saturation(saturation),// 0- 255
    .value_b(value_b), // 0- 255
    .valid_in(valid_rgbhsv),
//	 .pixel_addr(pixel_addr_interm),
    .erosion_mode(erosion_mode),
	 .dilation_mode(dilation_mode),
    .red_detect(red_detect),
    .green_detect(green_detect),
    .blue_detect(blue_detect),
    .grey_detect(grey_detect),
    .yellow_detect(yellow_detect)
//	 .pixel_addr_out(pixel_addr_obstacle)
);

// Find boundary of cursor box

// Highlight detected areas
wire [23:0] obstacle_high;
assign grey = green_out[7:1] + red_out[7:2] + blue_out[7:2]; //Grey = green/2 + red/4 + blue/4
assign obstacle_high  = red_detect ? {8'hff, 8'h0, 8'h0} : 
                        blue_detect ? {8'hCC, 8'hff, 8'hff} :
                        green_detect ? {8'h0, 8'hff, 8'h0} :
                        grey_detect ? {8'h0, 8'h33, 8'h66} :
                        yellow_detect ? {8'hff, 8'hff, 8'h0} :
                        {grey, grey, grey};

// Show bounding box
wire [23:0] new_image;
wire r_bb_active;
wire r2_bb_active;

wire g_bb_active;
wire g2_bb_active;

wire b_bb_active;
wire b2_bb_active;

wire gr_bb_active;
wire gr2_bb_active;

wire y_bb_active;
wire y2_bb_active;

assign r_bb_active = (x == r_left) | (x == r_right) | (y == r_top) | (y == r_bottom);
assign r2_bb_active = (x == r_left2) | (x == r_right2) | (y == r_top2) | (y == r_bottom2);

assign g_bb_active = (x == g_left) | (x == g_right) | (y == g_top) | (y == g_bottom);
assign g2_bb_active = (x == g_left2) | (x == g_right2) | (y == g_top) | (y == g_bottom);

assign b_bb_active = (x == b_left) | (x == b_right) | (y == b_top) | (y == b_bottom);
assign b2_bb_active = (x == b_left2) | (x == b_right2) | (y == b_top2) | (y == b_bottom2);

assign gr_bb_active = (x == gr_left) | (x == gr_right) | (y == gr_top) | (y == gr_bottom);
assign gr2_bb_active = (x == gr_left2) | (x == gr_right2) | (y == gr_top2) | (y == gr_bottom2);

assign y_bb_active = (x == y_left) | (x == y_right) | (y == y_top) | (y == y_bottom);
assign y2_bb_active = (x == y_left2) | (x == y_right2) | (y == y_top2) | (y == y_bottom2);

assign new_image = (r_bb_active|r2_bb_active) ? {8'hff, 8'h0, 8'h0} : 
                   (b_bb_active|b2_bb_active) ? {8'hCC, 8'hff, 8'hff} :
                   (g_bb_active|g2_bb_active) ? {8'h0, 8'hff, 8'h0} :
                   (gr_bb_active|gr2_bb_active) ? {8'h0, 8'h33, 8'h66} :
                   (y_bb_active|y2_bb_active) ? {8'hff, 8'hff, 8'h0} :
                   obstacle_high;

// Switch output pixels depending on mode switch
// Don't modify the start-of-packet word - it's a packet discriptor
// Don't modify data in non-video packets
assign {red_inter_out, green_inter_out, blue_inter_out} = (mode & ~sop_in & packet_video) ? new_image : {red_out,green_out,blue_out};


//Count valid pixels to get the image coordinates. Reset and detect packet type on Start of Packet.
reg [10:0] x, y;
reg packet_video;
always@(posedge clk) begin
    if (sop_in) begin
        x <= 11'h0;
        y <= 11'h0;
        packet_video <= (blue_out[3:0] == 3'h0);
    end
    else if (in_valid) begin
        if (x == IMAGE_W-1) begin
            x <= 11'h0;
            y <= y + 11'h1;
        end
        else begin
            x <= x + 11'h1;
        end
    end
end

//Find first and last red pixels
reg [10:0] r_x_min, r_y_min, r_x_max, r_y_max;
reg [10:0] r_x_min2, r_y_min2, r_x_max2, r_y_max2;
wire [11:0] r_x_mid = (r_x_min + r_x_max) >>1;
wire [11:0] r_y_mid =  (r_y_min + r_y_max)>>1;
wire [10:0] r_x_diff1 = r_x_mid - prev_x;
wire [10:0] r_x_diff2 = prev_x - r_x_mid;
wire [10:0] r_x_diff = (r_x_diff1[10]) ? r_x_diff2 : r_x_diff1;
wire [10:0] r_y_diff = prev_y - r_y_mid;

reg [10:0] g_x_min, g_y_min, g_x_max, g_y_max;
reg [10:0] g_x_min2, g_y_min2, g_x_max2, g_y_max2;
wire [11:0] g_x_mid = (g_x_min + g_x_max) >>1;
wire [11:0] g_y_mid =  (g_y_min + g_y_max)>>1;
wire [10:0] g_x_diff1 = g_x_mid - prev_x;
wire [10:0] g_x_diff2 = prev_x - g_x_mid;
wire [10:0] g_x_diff = (g_x_diff1[10]) ? g_x_diff2 : g_x_diff1;
wire [10:0] g_y_diff = prev_y - g_y_mid;

reg [10:0] b_x_min, b_y_min, b_x_max, b_y_max;
reg [10:0] b_x_min2, b_y_min2, b_x_max2, b_y_max2;
wire [11:0] b_x_mid = (b_x_min + b_x_max) >>1;
wire [11:0] b_y_mid =  (b_y_min + b_y_max)>>1;
wire [10:0] b_x_diff1 = b_x_mid - prev_x;
wire [10:0] b_x_diff2 = prev_x - b_x_mid;
wire [10:0] b_x_diff = (b_x_diff1[10]) ? b_x_diff2 : b_x_diff1;
wire [10:0] b_y_diff = prev_y - b_y_mid;

reg [10:0] gr_x_min, gr_y_min, gr_x_max, gr_y_max;
reg [10:0] gr_x_min2, gr_y_min2, gr_x_max2, gr_y_max2;
wire [11:0] gr_x_mid = (gr_x_min + gr_x_max) >>1;
wire [11:0] gr_y_mid =  (gr_y_min + gr_y_max)>>1;
wire [10:0] gr_x_diff1 = gr_x_mid - prev_x;
wire [10:0] gr_x_diff2 = prev_x - gr_x_mid;
wire [10:0] gr_x_diff = (gr_x_diff1[10]) ? gr_x_diff2 : gr_x_diff1;
wire [10:0] gr_y_diff = prev_y - gr_y_mid;

reg [10:0] y_x_min, y_y_min, y_x_max, y_y_max;
reg [10:0] y_x_min2, y_y_min2, y_x_max2, y_y_max2;
wire [11:0] y_x_mid = (y_x_min + y_x_max) >>1;
wire [11:0] y_y_mid =  (y_y_min + y_y_max)>>1;
wire [10:0] y_x_diff1 = y_x_mid - prev_x;
wire [10:0] y_x_diff2 = prev_x - y_x_mid;
wire [10:0] y_x_diff = (y_x_diff1[10]) ? y_x_diff2 : y_x_diff1;
wire [10:0] y_y_diff = prev_y - y_y_mid;

always@(posedge clk) begin
    if (valid_rgbhsv) begin
        if (red_detect) begin	//Update bounds when the pixel is red
				if ( (r_y_diff < 11'd60) |( r_x_diff < 11'd120)) begin
					if (prev_x < r_x_min) r_x_min <= prev_x;
					if (prev_x > r_x_max) r_x_max <= prev_x;
					if (prev_y < r_y_min) r_y_min <= prev_y;
					r_y_max <= prev_y;
				end
				else begin
					if (prev_x < r_x_min2) r_x_min2 <= prev_x;
					if (prev_x > r_x_max2) r_x_max2 <= prev_x;
					if (prev_y < r_y_min2) r_y_min2 <= prev_y;
					r_y_max2 <= prev_y;
				end
        end
        else if (blue_detect ) begin	//Update bounds when the pixel is blue
				if ( (b_y_diff < 11'd60) |( b_x_diff < 11'd120)) begin
					if (prev_x < b_x_min) b_x_min <= prev_x;
					if (prev_x >  b_x_max) b_x_max <= prev_x;
					if (prev_y < b_y_min) b_y_min <= prev_y;
					b_y_max <= prev_y;
				end
				else begin
					if (prev_x < b_x_min2) b_x_min2 <= prev_x;
					if (prev_x >  b_x_max2) b_x_max2 <= prev_x;
					if (prev_y < b_y_min2) b_y_min2 <= prev_y;
					b_y_max2 <= prev_y;
				end
        end 
        else if (green_detect ) begin	//Update bounds when the pixel is green
			  if ( (g_y_diff < 11'd60) |( g_x_diff < 11'd120)) begin
					if (prev_x < g_x_min) g_x_min <= prev_x;
					if (prev_x > g_x_max) g_x_max <= prev_x;
					if (prev_y < g_y_min) g_y_min <= prev_y;
					g_y_max <= prev_y;
				end
				else begin
					if (prev_x < g_x_min2) g_x_min2 <= prev_x;
					if (prev_x > g_x_max2) g_x_max2 <= prev_x;
					if (prev_y < g_y_min2) g_y_min2 <= prev_y;
					g_y_max2 <= prev_y;
				end
        end
        else if (grey_detect) begin	//Update bounds when the pixel is grey
				if ( (gr_y_diff < 11'd60) |( gr_x_diff < 11'd120)) begin
					if (prev_x < gr_x_min) gr_x_min <= prev_x;
					if (prev_x > gr_x_max) gr_x_max <= prev_x;
					if (prev_y < gr_y_min) gr_y_min <= prev_y;
					gr_y_max <= prev_y;
				end
				else begin
					if (prev_x < gr_x_min2) gr_x_min2 <= prev_x;
					if (prev_x >  gr_x_max2) gr_x_max2 <= prev_x;
					if (prev_y < gr_y_min2) gr_y_min2 <= prev_y;
					gr_y_max2 <= prev_y;
				end
        end
        else if (yellow_detect) begin	//Update bounds when the pixel is yellow
				if ( (gr_y_diff < 11'd60) |( gr_x_diff < 11'd120)) begin
					if (prev_x < y_x_min) y_x_min <= prev_x;
					if (prev_x > y_x_max) y_x_max <= prev_x;
					if (prev_y < y_y_min) y_y_min <= prev_y;
					y_y_max <= prev_y;
				end
				else begin
					if (prev_x < y_x_min2) y_x_min2 <= prev_x;
					if (prev_x > y_x_max2) y_x_max2 <= prev_x;
					if (prev_y < y_y_min2) y_y_min2 <= prev_y;
					y_y_max2 <= prev_y;
				end
        end
		  if (value_b[7] == 1'b1) begin
		      bright_pix_count <= bright_pix_count + 20'd1;
		  end
	 end
	 if (sop_in) begin	//Reset bounds on start of packet
		 r_x_min <= IMAGE_W-11'h1; r_x_min2 <= IMAGE_W-11'h1;
		 b_x_min <= IMAGE_W-11'h1; b_x_min2 <= IMAGE_W-11'h1;
		 g_x_min <= IMAGE_W-11'h1; g_x_min2 <= IMAGE_W-11'h1;
		 gr_x_min <= IMAGE_W-11'h1; gr_x_min2 <= IMAGE_W-11'h1;
		 y_x_min <= IMAGE_W-11'h1; y_x_min2 <= IMAGE_W-11'h1;
		 
		 r_x_max <= 0; r_x_max2 <= 0;
		 b_x_max <= 0; b_x_max2 <= 0;
		 g_x_max <= 0; g_x_max2 <= 0;
		 gr_x_max <= 0;gr_x_max2 <= 0;
		 y_x_max <= 0; y_x_max2 <= 0;
		 
		 r_y_min <= IMAGE_H-11'h1; r_y_min2 <= IMAGE_H-11'h1;
		 b_y_min <= IMAGE_H-11'h1; b_y_min2 <= IMAGE_H-11'h1;
		 g_y_min <= IMAGE_H-11'h1; g_y_min2 <= IMAGE_H-11'h1;
		 gr_y_min <= IMAGE_H-11'h1; gr_y_min2 <= IMAGE_H-11'h1;
		 y_y_min <= IMAGE_H-11'h1; y_y_min2 <= IMAGE_H-11'h1;
		 
		 r_y_max <= 0; r_y_max2 <= 0;
		 b_y_max <= 0;b_y_max2 <= 0;
		 g_y_max <= 0; g_y_max2 <= 0;
		 gr_y_max <= 0;gr_y_max2 <= 0;
		 y_y_max <= 0; y_y_max2 <= 0;
		 
		 bright_pix_count <= 20'd0;
	 end
end

//Process bounding box at the end of the frame.
reg [4:0] msg_state;
reg [10:0] r_left, r_right, r_top, r_bottom;
reg [10:0] r_left2, r_right2, r_top2, r_bottom2;

reg [10:0] g_left, g_right, g_top, g_bottom;
reg [10:0] g_left2, g_right2, g_top2, g_bottom2;

reg [10:0] b_left, b_right, b_top, b_bottom;
reg [10:0] b_left2, b_right2, b_top2, b_bottom2;

reg [10:0] gr_left, gr_right, gr_top, gr_bottom;
reg [10:0] gr_left2, gr_right2, gr_top2, gr_bottom2;

reg [10:0] y_left, y_right, y_top, y_bottom;
reg [10:0] y_left2, y_right2, y_top2, y_bottom2;

reg [19:0] bright_pix_count_reg;
reg [7:0] frame_count;


always@(posedge clk) begin
    if (eop_in & in_valid & packet_video) begin  //Ignore non-video packets
        //Latch edges for display overlay on next frame
        r_left <= r_x_min;
        r_right <= r_x_max;
        r_top <= r_y_min;
        r_bottom <= r_y_max;

        r_left2 <= r_x_min2;
        r_right2 <= r_x_max2;
        r_top2 <= r_y_min2;
        r_bottom2 <= r_y_max2;

        g_left <= g_x_min;
        g_right <= g_x_max;
        g_top <= g_y_min;
        g_bottom <= g_y_max;	
        
        g_left2 <= g_x_min2;
        g_right2 <= g_x_max2;
        g_top2 <= g_y_min2;
        g_bottom2 <= g_y_max2;	

        b_left <= b_x_min;
        b_right <= b_x_max;
        b_top <= b_y_min;
        b_bottom <= b_y_max;	

		b_left2 <= b_x_min2;
        b_right2 <= b_x_max2;
        b_top2 <= b_y_min2;
        b_bottom2 <= b_y_max2;	
		  
        gr_left <= gr_x_min;
        gr_right <= gr_x_max;
        gr_top <= gr_y_min;
        gr_bottom <= gr_y_max;	

		gr_left2 <= gr_x_min2;
        gr_right2 <= gr_x_max2;
        gr_top2 <= gr_y_min2;
        gr_bottom2 <= gr_y_max2;
		  
        y_left <= y_x_min;
        y_right <= y_x_max;
        y_top <= y_y_min;
        y_bottom <= y_y_max;

        y_left2 <= y_x_min2;
        y_right2 <= y_x_max2;
        y_top2 <= y_y_min2;
        y_bottom2 <= y_y_max2;	

        bright_pix_count_reg <= bright_pix_count;
		  
        //Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
        frame_count <= frame_count - 8'd1;
        
        if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin
            msg_state <= 5'd1;
            frame_count <= MSG_INTERVAL-8'd1;
        end
    end

    //Cycle through message writer states once started
    if (msg_state != 5'd0)
        begin
            if (msg_state == 5'd22)  msg_state <= 5'd0;
            else msg_state <= msg_state + 5'd1;
        end
end
	
//Generate output messages for CPU
reg [31:0] msg_buf_in; 
wire [31:0] msg_buf_out;
reg msg_buf_wr;
wire msg_buf_rd, msg_buf_flush;
wire [7:0] msg_buf_size;
wire msg_buf_empty;

`define START_MSG_ID "RBB"

always@(*) begin	//Write words to FIFO as state machine advances
    case(msg_state)
        5'd0: begin
            msg_buf_in = 32'b0;
            msg_buf_wr = 1'b0;
        end
        5'd1: begin
            msg_buf_in = `START_MSG_ID;	//Message ID
            msg_buf_wr = 1'b1;
        end
        // Red
        5'd2: begin
            msg_buf_in = {5'b0, (r_left + r_right) >>1 , 5'b0, (r_top + r_bottom)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd3: begin
            msg_buf_in = {5'b0, r_bottom - r_top, 5'b0, r_right - r_left}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
        5'd4: begin
            msg_buf_in = {5'b0, (r_left2 + r_right2) >>1 , 5'b0, (r_top2 + r_bottom2)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd5: begin
            msg_buf_in = {5'b0, r_bottom2 - r_top2, 5'b0, r_right2 - r_left2}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
        // Green
        5'd6: begin
            msg_buf_in = {5'b0, (g_left + g_right) >>1 , 5'b0, (g_top + g_bottom)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd7: begin
            msg_buf_in = {5'b0, g_bottom - g_top, 5'b0, g_right - g_left}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
        5'd8: begin
            msg_buf_in = {5'b0, (g_left2 + g_right2) >>1 , 5'b0, (g_top2 + g_bottom2)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd9: begin
            msg_buf_in = {5'b0, g_bottom2 - g_top2, 5'b0, g_right2 - g_left2}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
        // Blue
        5'd10: begin
            msg_buf_in = {5'b0, (b_left + b_right) >>1 , 5'b0, (b_top + b_bottom)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd11: begin
            msg_buf_in = {5'b0, b_bottom - b_top, 5'b0, b_right - b_left}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
		  
		5'd12: begin
            msg_buf_in = {5'b0, (b_left2 + b_right2) >>1 , 5'b0, (b_top2 + b_bottom2)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd13: begin
            msg_buf_in = {5'b0, b_bottom2 - b_top2, 5'b0, b_right2 - b_left2}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
        // Grey
        5'd14: begin
            msg_buf_in = {5'b0, (gr_left + gr_right) >>1 , 5'b0, (gr_top + gr_bottom)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd15: begin
            msg_buf_in = {5'b0, gr_bottom - gr_top, 5'b0, gr_right - gr_left}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
		  
        5'd16: begin
            msg_buf_in = {5'b0, (gr_left2 + gr_right2) >>1 , 5'b0, (gr_top2 + gr_bottom2)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd17: begin
            msg_buf_in = {5'b0, gr_bottom2 - gr_top2, 5'b0, gr_right2 - gr_left2}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
        // Yellow
        5'd18: begin
            msg_buf_in = {5'b0, (y_left + y_right) >>1 , 5'b0, (y_top + y_bottom)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd19: begin
            msg_buf_in = {5'b0, y_bottom - y_top, 5'b0, y_right - y_left}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
        5'd20: begin
            msg_buf_in = {5'b0, (y_left2 + y_right2) >>1 , 5'b0, (y_top2 + y_bottom2)>>1};	//Top left coordinate
            msg_buf_wr = 1'b1;
        end
        5'd21: begin
            msg_buf_in = {5'b0, y_bottom2 - y_top2, 5'b0, y_right2 - y_left2}; //Bottom right coordinate
            msg_buf_wr = 1'b1;
        end
        // Bright Pix Count
        5'd22: begin
            msg_buf_in = {12'b0, bright_pix_count_reg}; //Bright pixel count
            msg_buf_wr = 1'b1;
        end
        default: begin
            msg_buf_in = 32'b0;
            msg_buf_wr = 1'b0;
        end
    endcase
end


//Output message FIFO
MSG_FIFO	MSG_FIFO_inst (
    .clock (clk), 
    .data (msg_buf_in),
    .rdreq (msg_buf_rd),
    .sclr (~reset_n | msg_buf_flush),
    .wrreq (msg_buf_wr),
    .q (msg_buf_out),
    .usedw (msg_buf_size),
    .empty (msg_buf_empty)
);



STREAM_REG #(.DATA_WIDTH(26)) out_reg (
    .clk(clk),
    .rst_n(reset_n),
    .ready_out(out_ready),
    .valid_out(source_valid),
    .data_out({source_data,source_sop,source_eop}),
    .ready_in(source_ready),
    .valid_in(in_valid),
    .data_in({red_inter_out, green_inter_out, blue_inter_out, sop_in, eop_in})
);


/////////////////////////////////
/// Memory-mapped port		 /////
/////////////////////////////////

// Addresses
`define REG_STATUS    			0
`define READ_MSG    				1
`define READ_ID    				2
`define REG_BBCOL					3

//Status register bits
// 31:16 - unimplemented
// 15:8 - number of words in message buffer (read only)
// 7:5 - unused
// 4 - flush message buffer (write only - read as 0)
// 3:0 - unused


// Process write

reg  [7:0]   reg_status;
reg	 [23:0]  bb_col;

always @ (posedge clk)
begin
    if (~reset_n)
    begin
        reg_status <= 8'b0;
        bb_col <= BB_COL_DEFAULT;
    end
    else begin
        if(s_chipselect & s_write) begin
            if      (s_address == `REG_STATUS)	reg_status <= s_writedata[7:0];
            if      (s_address == `REG_BBCOL)	bb_col <= s_writedata[23:0];
        end
    end
end


//Flush the message buffer if 1 is written to status register bit 4
assign msg_buf_flush = (s_chipselect & s_write & (s_address == `REG_STATUS) & s_writedata[4]);


// Process reads
reg read_d; //Store the read signal for correct updating of the message buffer

// Copy the requested word to the output port when there is a read.
always @ (posedge clk)
begin
    if (~reset_n) begin
        s_readdata <= {32'b0};
        read_d <= 1'b0;
    end

    else if (s_chipselect & s_read) begin
        if   (s_address == `REG_STATUS) s_readdata <= {16'b0,msg_buf_size,reg_status};
        if   (s_address == `READ_MSG) s_readdata <= {msg_buf_out};
        if   (s_address == `READ_ID) s_readdata <= 32'h1234EEE2;
        if   (s_address == `REG_BBCOL) s_readdata <= {8'h0, bb_col};
    end

    read_d <= s_read;
end

//Fetch next word from message buffer after read from READ_MSG
assign msg_buf_rd = s_chipselect & s_read & ~read_d & ~msg_buf_empty & (s_address == `READ_MSG);

endmodule