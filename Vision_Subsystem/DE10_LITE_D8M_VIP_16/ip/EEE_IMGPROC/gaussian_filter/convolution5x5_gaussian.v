
module convolution5x5_gaussian(
	input clk,
	input rst_n,
	input [DATA_WIDTH - 1:0] i_pixel,
	input i_pixel_valid,
	output [DATA_WIDTH - 1:0] tap0,
	output [DATA_WIDTH - 1:0] tap1,
	output [DATA_WIDTH - 1:0] tap2,
	output [DATA_WIDTH - 1:0] tap3,
	output [DATA_WIDTH * 25 - 1:0] o_pixel,
	output [DATA_WIDTH - 1:0] pixel0_check,
	output [DATA_WIDTH - 1:0] pixel1_check, 
	output [DATA_WIDTH - 1:0] pixel2_check, 
	output [DATA_WIDTH - 1:0] pixel3_check, 
	output [DATA_WIDTH - 1:0] pixel4_check, 
	output [DATA_WIDTH - 1:0] pixel5_check, 
	output [DATA_WIDTH - 1:0] pixel6_check, 
	output [DATA_WIDTH - 1:0] pixel7_check, 
	output [DATA_WIDTH - 1:0] pixel8_check,
	output [18:0] pixelCounter1,
	output [9:0] pixelRowCounter1,
	output [11:0] pixelStartUpCounter_check
);

localparam width = 640;
localparam height = 480;
// Parameter Declaration(s)
parameter DATA_WIDTH = 8;

reg [DATA_WIDTH - 1:0] pixel0, pixel1, pixel2, pixel3, pixel4;
reg [DATA_WIDTH - 1:0] pixel5, pixel6, pixel7, pixel8, pixel9;
reg [DATA_WIDTH - 1:0] pixel10, pixel11, pixel12, pixel13, pixel14;
reg [DATA_WIDTH - 1:0] pixel15, pixel16, pixel17, pixel18, pixel19;
reg [DATA_WIDTH - 1:0] pixel20, pixel21, pixel22, pixel23, pixel24;
reg [18:0] pixelCounter;
reg [9:0] pixelRowCounter;
reg [11:0] pixelStartUpCounter;

assign pixel0_check = pixel0;
assign pixel1_check = pixel1;
assign pixel2_check = pixel2;
assign pixel3_check = pixel3;
assign pixel4_check = pixel4;
assign pixel5_check = pixel5;
assign pixel6_check = pixel6;
assign pixel7_check = pixel7;
assign pixel8_check = pixel8;
assign pixelStartUpCounter_check = pixelStartUpCounter;

initial
	begin
		pixel0<= 8'd0;
		pixel1<= 8'd0;
		pixel2<= 8'd0;
		pixel3<= 8'd0;
		pixel4<= 8'd0;
		pixel5<= 8'd0;
		pixel6<= 8'd0;
		pixel7<= 8'd0;
		pixel8<= 8'd0;
		pixel9<= 8'd0;
		pixel10<= 8'd0;
		pixel11<= 8'd0;
		pixel12<= 8'd0;
		pixel13<= 8'd0;
		pixel14<= 8'd0;
		pixel15<= 8'd0;
		pixel16<= 8'd0;
		pixel17<= 8'd0;
		pixel18<= 8'd0;
		pixel19<= 8'd0;
		pixel20<= 8'd0;
		pixel21<= 8'd0;
		pixel22<= 8'd0;
		pixel23<= 8'd0;
		pixel24<= 8'd0;
		
	end
	

// FSM
reg data_ready;
reg [1:0] state;
localparam START_UP = 1'b0;
localparam FILTER_ON = 1'b1;
initial begin
	data_ready <= 1'b1;
	state <= START_UP;
end

always @(posedge clk)
	begin
		if (~rst_n)
			begin
				state <= START_UP;
				data_ready <= 1'b1;
				pixelStartUpCounter <= 6'd0;
			end
		else begin
			if(state == START_UP) begin
				if (pixelStartUpCounter == width * 4 +2) 
					begin
						state <= FILTER_ON;
						data_ready <= 1'b0;
						pixelStartUpCounter <= 6'd0;
					end
				else if (i_pixel_valid) pixelStartUpCounter <= pixelStartUpCounter + 6'd1;
			end
					
		end
	end
	
	
assign pixelCounter1 = pixelCounter; 
assign pixelRowCounter1 = pixelRowCounter;

// Pixel Counters (Total and Row)
always @(posedge clk)
	begin
		if(~rst_n | ((pixelCounter == width * height - 1 | data_ready) & i_pixel_valid) )
			pixelCounter <= 4'd0;
		else if (i_pixel_valid)
			pixelCounter <= pixelCounter + 4'd1;
	end

always @(posedge clk)
	begin
		if( ~rst_n | (pixelRowCounter == width - 1 | data_ready ) & i_pixel_valid) 
			pixelRowCounter <= 4'd0;
		else if (i_pixel_valid)	
			pixelRowCounter <= pixelRowCounter + 4'd1;
	end

wire pixelEnable;
assign pixelEnable = ~(pixelRowCounter == 0 | pixelRowCounter == 1 | pixelRowCounter == width - 2 | pixelRowCounter == width - 1 | data_ready | (pixelCounter > width * (height-2) & pixelCounter < width * height ));	

line_buffer5x5_8bit lbuff (
	.clock ( clk ),
	.clken ( i_pixel_valid ),
	.aclr ( ~rst_n), 
	.shiftin ( i_pixel ),
	.taps0x ( tap0 ),
	.taps1x ( tap1 ),
	.taps2x ( tap2 ),
	.taps3x ( tap3 )
	);
	
			
always @(posedge clk)
	begin
		if (i_pixel_valid) begin	
			pixel24<= i_pixel; 
			pixel23<= pixel24;
			pixel22<= pixel23;
			pixel21<= pixel22;
			pixel20<= pixel21;
			
			pixel19<= tap0;
			pixel18<= pixel19;
			pixel17<= pixel18;
			pixel16<= pixel17;
			pixel15<= pixel16; 
			
			pixel14<= tap1;
			pixel13<= pixel14;
			pixel12<= pixel13;
			pixel11<= pixel12;
			pixel10<= pixel11;	

         pixel9<= tap2;			
			pixel8<= pixel9;
			pixel7<= pixel8;
			pixel6<= pixel7;
			pixel5<= pixel6; 
			
			pixel4<= tap3;
			pixel3<= pixel4;
			pixel2<= pixel3;
			pixel1<= pixel2;
			pixel0<= pixel1;
		end
	end
wire [13:0] pixel24_mult = (pixel24 * 8'd24 ) >> 10;
wire [13:0] pixel23_mult = (pixel23 * 8'd35 ) >> 10;
wire [13:0] pixel22_mult = (pixel22 * 8'd39 ) >> 10;
wire [13:0] pixel21_mult = (pixel21 * 8'd35 ) >> 10;
wire [13:0] pixel20_mult = (pixel20 * 8'd24 ) >> 10;

wire [13:0] pixel19_mult = (pixel19 * 8'd35 ) >> 10;
wire [13:0] pixel18_mult = (pixel18 * 8'd50 ) >> 10;
wire [13:0] pixel17_mult = (pixel17 * 8'd57 ) >> 10;
wire [13:0] pixel16_mult = (pixel16 * 8'd50 ) >> 10;
wire [13:0] pixel15_mult = (pixel15 * 8'd35 ) >> 10;

wire [13:0] pixel14_mult = (pixel14 * 8'd39 ) >> 10;
wire [13:0] pixel13_mult = (pixel13 * 8'd57 ) >> 10;
wire [13:0] pixel12_mult = (pixel12 * 8'd64 ) >> 10;
wire [13:0] pixel11_mult = (pixel11 * 8'd57 ) >> 10;
wire [13:0] pixel10_mult = (pixel10 * 8'd39 ) >> 10;

wire [13:0] pixel9_mult = (pixel9 * 8'd35 ) >> 10;
wire [13:0] pixel8_mult = (pixel8 * 8'd50 ) >> 10;
wire [13:0] pixel7_mult = (pixel7 * 8'd57 ) >> 10;
wire [13:0] pixel6_mult = (pixel6 * 8'd50 ) >> 10;
wire [13:0] pixel5_mult = (pixel5 * 8'd35 ) >> 10;

wire [13:0] pixel4_mult = (pixel4 * 8'd24 ) >> 10;
wire [13:0] pixel3_mult = (pixel3 * 8'd35 ) >> 10;
wire [13:0] pixel2_mult = (pixel2 * 8'd39 ) >> 10;
wire [13:0] pixel1_mult = (pixel1 * 8'd35 ) >> 10;
wire [13:0] pixel0_mult = (pixel0 * 8'd24 ) >> 10;

assign o_pixel = {(pixelEnable)?pixel24_mult + pixel23_mult + pixel22_mult + pixel21_mult + pixel20_mult + 
										  pixel19_mult + pixel18_mult + pixel17_mult + pixel16_mult + pixel15_mult + 
										  pixel14_mult + pixel13_mult + pixel12_mult + pixel11_mult + pixel10_mult + 
										  pixel9_mult + pixel8_mult + pixel7_mult + pixel6_mult + pixel5_mult + 
										  pixel4_mult + pixel3_mult + pixel2_mult + pixel1_mult + pixel0_mult:8'd0};
endmodule