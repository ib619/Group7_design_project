
module convolution3x3_dilation(
	input clk,
	input rst_n,
	input [DATA_WIDTH - 1:0] i_pixel,
	input i_pixel_valid,
	output [DATA_WIDTH - 1:0] tap0,
	output [DATA_WIDTH - 1:0] tap1,
	output [DATA_WIDTH - 1:0] o_pixel,
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
	output [10:0] pixelStartUpCounter_check
);

localparam width = 640;
localparam height = 480;
// Parameter Declaration(s)
parameter DATA_WIDTH = 8;

reg [DATA_WIDTH - 1:0] pixel0, pixel1, pixel2, pixel3, pixel4, pixel5, pixel6, pixel7, pixel8;
reg [18:0] pixelCounter;
reg [9:0] pixelRowCounter;
reg [10:0] pixelStartUpCounter;

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
		pixel0<= 1'd0;
		pixel1<= 1'd0;
		pixel2<= 1'd0;
		pixel3<= 1'd0;
		pixel4<= 1'd0;
		pixel5<= 1'd0;
		pixel6<= 1'd0;
		pixel7<= 1'd0;
		pixel8<= 1'd0;
		
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
				pixelStartUpCounter <= 5'd0;
			end
		else begin
			if(state == START_UP) begin
				if (pixelStartUpCounter == width * 2 + 1) 
					begin
						state <= FILTER_ON;
						data_ready <= 1'b0;
						pixelStartUpCounter <= 5'd0;
					end
				else if (i_pixel_valid) pixelStartUpCounter <= pixelStartUpCounter + 5'd1;
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
			pixel8<= i_pixel;
			pixel7<= pixel8;
			pixel6<= pixel7;
			pixel5<=  tap0; 
			pixel4<= pixel5;
			pixel3<= pixel4;
			pixel2<= tap1;
			pixel1<= pixel2;
			pixel0<= pixel1;
		end
	end

assign o_pixel = pixel7 | pixel5 | pixel4 | pixel3 | pixel1;

endmodule