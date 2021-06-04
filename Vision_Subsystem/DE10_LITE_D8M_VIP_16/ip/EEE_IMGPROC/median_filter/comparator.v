module comparator(
	input [DATA_WIDTH - 1:0] pixel0,
	input [DATA_WIDTH - 1:0] pixel1, 
	output [DATA_WIDTH - 1:0] high, 
	output [DATA_WIDTH - 1:0] low
);

parameter DATA_WIDTH = 8;

assign high = (pixel0 >= pixel1) ? pixel0 : pixel1;
assign low = (pixel0 >= pixel1) ? pixel1 : pixel0;

endmodule