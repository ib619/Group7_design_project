module morph_erosion_filter5x5 (
	input clk,
	input rst_n,
	input i_pixel,
	input i_pixel_valid, // Both in_valid and packet video
	output o_convolved_data
//	output [15:0] o_convolved_data
);


separableconvolution5x5_erosion #(.DATA_WIDTH(1)) filter(
	.clk(clk),
	.rst_n(rst_n),
	.i_pixel(i_pixel),
	.i_pixel_valid(i_pixel_valid),
	.o_pixel(o_convolved_data)
);

endmodule