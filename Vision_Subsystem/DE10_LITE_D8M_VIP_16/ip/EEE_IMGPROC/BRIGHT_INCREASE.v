module BRIGHT_INCREASE(

	input [7:0] RGB_IN,
	
	input mode,

	output reg [7:0] RGB_OUT
	
);

wire [10:0] RGB_GAIN = (mode) ? RGB_IN + 8'd32 : RGB_IN ;
//wire [10:0] RGB_GAIN = RGB_IN + 10'd64 ;

always @(*)
	begin 
		if (RGB_GAIN > 8'hFF) begin
			RGB_OUT = 8'hFF;
		end else begin
			RGB_OUT = RGB_GAIN[7:0];
		end
	end
endmodule
