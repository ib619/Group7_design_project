module separableconvolution5x5_gaussian(
	input clk,
	input rst_n,
	input [7:0] i_pixel,
	input i_pixel_valid,
	output [7:0] tap0,
	output [7:0] tap1,
	output [7:0] tap2,
	output [7:0] tap3,
	output [7:0] o_pixel,
	output [11:0] pixel1_check, 
	output [11:0] pixel2_check,  
	output [11:0] pixel3_check, 
	output pixelEnable_check,
	output [18:0] pixelCounter1,
	output [9:0] pixelRowCounter1,
	output [11:0] pixelStartUpCounter_check,
	output [11:0] first_conv_check,
	output [11:0] second_conv_check,
	output [7:0] shift_out_check
);

localparam width = 9;
localparam height = 6;
// Parameter Declaration(s)
parameter DATA_WIDTH = 8;

reg [DATA_WIDTH + 3:0] pixel1, pixel2, pixel3, pixel4, pixel5;
reg [DATA_WIDTH + 3:0] first_conv_out;
wire [DATA_WIDTH + 3:0] first_conv_bef_div;
//wire [DATA_WIDTH + 3:0] second_conv_out;
wire [DATA_WIDTH + 3:0] second_conv_bef_div;

reg [18:0] pixelCounter;
reg [9:0] pixelRowCounter;
reg [11:0] pixelStartUpCounter;
wire pixelEnable;

assign pixel1_check = pixel1;
assign pixel2_check = pixel2;
assign pixel3_check = pixel3;
assign pixelEnable_check = pixelEnable;
assign first_conv_check = first_conv_out;
assign second_conv_check = second_conv_bef_div;

assign pixelStartUpCounter_check = pixelStartUpCounter;

initial
	begin
		pixel1<= 12'd0;
		pixel2<= 12'd0;
		pixel3<= 12'd0;
		
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
				pixelStartUpCounter <= 11'd0;
			end
		else begin
			if(state == START_UP) begin
				if (pixelStartUpCounter == width * 4 + 3) 
					begin
						state <= FILTER_ON;
						data_ready <= 1'b0;
						pixelStartUpCounter <= 11'd0;
					end
				else if (i_pixel_valid) pixelStartUpCounter <= pixelStartUpCounter + 11'd1;
			end
					
		end
	end
	
	
assign pixelCounter1 = pixelCounter; 
assign pixelRowCounter1 = pixelRowCounter;

// Pixel Counters (Total and Row)
always @(posedge clk)
	begin
		if(~rst_n | ((pixelCounter == width * height - 1 | data_ready) & i_pixel_valid) )
			pixelCounter <= 19'd0;
		else if (i_pixel_valid)
			pixelCounter <= pixelCounter + 19'd1;
	end

always @(posedge clk)
	begin
		if( ~rst_n | (pixelRowCounter == width - 1 | data_ready ) & i_pixel_valid) 
			pixelRowCounter <= 10'd0;
		else if (i_pixel_valid)	
			pixelRowCounter <= pixelRowCounter + 10'd1;
	end


assign pixelEnable = ~(pixelRowCounter == 0 | pixelRowCounter == 1 | pixelRowCounter == width - 2 | pixelRowCounter == width - 1 | data_ready | (pixelCounter > width * (height-4) & pixelCounter < width * height ));	

line_buffer5x5_8bit lbuff3 (
	.clock ( clk ),
	.clken ( i_pixel_valid ),
	.aclr ( ~rst_n), 
	.shiftin ( i_pixel ),
	.shiftout (shift_out_check),
	.taps0x ( tap0 ),
	.taps1x ( tap1 ),
	.taps2x ( tap2 ),
	.taps3x ( tap3 )
	);
	
	
always @(posedge clk)
	begin
		if (i_pixel_valid) begin		
			pixel1<= first_conv_out;
			pixel2<= pixel1;
			pixel3<= pixel2;
			pixel4<= pixel3;
			pixel5<= pixel4;			
			first_conv_out <= first_conv_bef_div >> 4; // 1/16 [ 1 4 6 4 1]
			
		end
	end
assign first_conv_bef_div = i_pixel + tap0 * 4 + tap1 * 6 + tap2 * 4 + tap3;
assign second_conv_bef_div = pixel1 + pixel2 * 4 + pixel3 * 6 + pixel4 * 4 + pixel5;
assign o_pixel = (pixelEnable)? (second_conv_bef_div >> 4): 7'd0;
endmodule