
module separableconvolution3x3_erosion(
	input clk,
	input rst_n,
	input [DATA_WIDTH - 1:0] i_pixel,
	input i_pixel_valid,
	output [DATA_WIDTH - 1:0] tap0,
	output [DATA_WIDTH - 1:0] tap1,
	output [DATA_WIDTH - 1:0] o_pixel,
	output [DATA_WIDTH - 1:0] pixel1_check, 
	output [DATA_WIDTH - 1:0] pixel2_check, 
	output [DATA_WIDTH - 1:0] pixel3_check, 
	output [18:0] pixelCounter1,
	output [9:0] pixelRowCounter1,
	output [10:0] pixelStartUpCounter_check
);

localparam width = 640;
localparam height = 480;
// Parameter Declaration(s)
parameter DATA_WIDTH = 1;

reg pixel1, pixel2, pixel3;
reg first_conv_out;

reg [18:0] pixelCounter;
reg [9:0] pixelRowCounter;
reg [10:0] pixelStartUpCounter;

assign pixel1_check = pixel1;
assign pixel2_check = pixel2;
assign pixel3_check = pixel3;

assign pixelStartUpCounter_check = pixelStartUpCounter;

initial
	begin
		pixel1<= 1'd0;
		pixel2<= 1'd0;
		pixel3<= 1'd0;
		
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
				if (pixelStartUpCounter == width * 2 + 2) 
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

wire pixelEnable;
assign pixelEnable = ~(pixelRowCounter == 0 | pixelRowCounter == width - 1 | data_ready | (pixelCounter > width * (height-2) & pixelCounter < width * height ));	

line_buffer3x3_1bit lbuff (
	.clock ( clk ),
	.clken ( i_pixel_valid ),
	.aclr ( ~rst_n), 
	.shiftin ( i_pixel ),
	.taps0x ( tap0 ),
	.taps1x ( tap1 )
	);
	

always @(posedge clk)
	begin
		if (i_pixel_valid) begin		
			pixel1<= first_conv_out;
			pixel2<= pixel1;
			pixel3<= pixel2;
			first_conv_out <= i_pixel & tap0 & tap1 ; 
		end
	end

assign o_pixel = (pixelEnable)? pixel1 & pixel2 & pixel3 :8'd0;
endmodule