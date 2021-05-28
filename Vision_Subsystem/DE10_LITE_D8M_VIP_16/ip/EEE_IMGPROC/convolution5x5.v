
module convolution5x5(
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
	output [8:0] pixelCounter1,
	output [3:0] pixelRowCounter1,
	output [6:0] pixelStartUpCounter_check
);

localparam width = 9;
localparam height = 5;
// Parameter Declaration(s)
parameter DATA_WIDTH = 8;

reg [DATA_WIDTH - 1:0] pixel0, pixel1, pixel2, pixel3, pixel4;
reg [DATA_WIDTH - 1:0] pixel5, pixel6, pixel7, pixel8, pixel9;
reg [DATA_WIDTH - 1:0] pixel10, pixel11, pixel12, pixel13, pixel14;
reg [DATA_WIDTH - 1:0] pixel15, pixel16, pixel17, pixel18, pixel19;
reg [DATA_WIDTH - 1:0] pixel20, pixel21, pixel22, pixel23, pixel24;
reg [8:0] pixelCounter;
reg [3:0] pixelRowCounter;
reg [6:0] pixelStartUpCounter;

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

line_buffer3 lbuff (
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

assign o_pixel = {(pixelEnable)?pixel24:8'd0,  
					   (pixelEnable)?pixel23:8'd0,
					   (pixelEnable)?pixel22:8'd0, 
					   (pixelEnable)?pixel21:8'd0, 
					   (pixelEnable)?pixel20:8'd0,
						(pixelEnable)?pixel19:8'd0,
						(pixelEnable)?pixel18:8'd0,
				     	(pixelEnable)?pixel17:8'd0,
					   (pixelEnable)?pixel16:8'd0, 
					   (pixelEnable)?pixel15:8'd0, 
					   (pixelEnable)?pixel14:8'd0,  
					   (pixelEnable)?pixel13:8'd0,
					   (pixelEnable)?pixel12:8'd0, 
					   (pixelEnable)?pixel11:8'd0, 
					   (pixelEnable)?pixel10:8'd0,
						(pixelEnable)?pixel9:8'd0,
                  (pixelEnable)?pixel8:8'd0,
				     	(pixelEnable)?pixel7:8'd0,
					   (pixelEnable)?pixel6:8'd0, 
					   (pixelEnable)?pixel5:8'd0, 
					   (pixelEnable)?pixel4:8'd0,  
					   (pixelEnable)?pixel3:8'd0,
					   (pixelEnable)?pixel2:8'd0, 
					   (pixelEnable)?pixel1:8'd0, 
					   (pixelEnable)?pixel0:8'd0};
endmodule