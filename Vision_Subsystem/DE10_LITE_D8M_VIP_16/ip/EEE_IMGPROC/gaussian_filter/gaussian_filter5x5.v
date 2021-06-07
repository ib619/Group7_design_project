module gaussian_filter5x5 (
	input clk,
	input rst_n,
	input [7:0] i_pixel_red,
	input [7:0] i_pixel_blue,
	input [7:0] i_pixel_green,
	input i_pixel_valid, // Both in_valid and packet video
	output [7:0] o_convolved_data_red,
	output [7:0] o_convolved_data_blue,
	output [7:0] o_convolved_data_green
);

separableconvolution5x5_gaussian #(.DATA_WIDTH(8)) filter_red(
	.clk(clk),
	.rst_n(rst_n),
	.i_pixel(i_pixel_red),
	.i_pixel_valid(i_pixel_valid),
	.o_pixel(o_convolved_data_red)
);

separableconvolution5x5_gaussian #(.DATA_WIDTH(8)) filter_blue(
	.clk(clk),
	.rst_n(rst_n),
	.i_pixel(i_pixel_blue),
	.i_pixel_valid(i_pixel_valid),
	.o_pixel(o_convolved_data_blue)
);

separableconvolution5x5_gaussian #(.DATA_WIDTH(8)) filter_green(
	.clk(clk),
	.rst_n(rst_n),
	.i_pixel(i_pixel_green),
	.i_pixel_valid(i_pixel_valid),
	.o_pixel(o_convolved_data_green)
);

//convolution5x5_gaussian #(.DATA_WIDTH(8)) filter_red(
//	.clk(clk),
//	.rst_n(rst_n),
//	.i_pixel(i_pixel_red),
//	.i_pixel_valid(i_pixel_valid),
//	.o_pixel(o_convolved_data_red)
//);
//
//convolution5x5_gaussian #(.DATA_WIDTH(8)) filter_blue(
//	.clk(clk),
//	.rst_n(rst_n),
//	.i_pixel(i_pixel_blue),
//	.i_pixel_valid(i_pixel_valid),
//	.o_pixel(o_convolved_data_blue)
//);
//
//convolution5x5_gaussian #(.DATA_WIDTH(8)) filter_green(
//	.clk(clk),
//	.rst_n(rst_n),
//	.i_pixel(i_pixel_green),
//	.i_pixel_valid(i_pixel_valid),
//	.o_pixel(o_convolved_data_green)
//);


endmodule