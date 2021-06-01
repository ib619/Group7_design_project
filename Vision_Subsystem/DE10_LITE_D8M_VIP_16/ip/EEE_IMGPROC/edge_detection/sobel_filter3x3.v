module sobel_filter3x3 (
	input clk,
	input rst_n,
	input [7:0] i_pixel,
	input i_pixel_valid, // Both in_valid and packet video
	output o_convolved_data
);

wire [10:0] o_gx;
wire [10:0] o_gy;

separableconvolution3x3_gx #(.DATA_WIDTH(8)) filter_gx(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(i_pixel),
    .i_pixel_valid(i_pixel_valid),
    .o_pixel(o_gx)
);

separableconvolution3x3_gy #(.DATA_WIDTH(8)) filter_gy(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(i_pixel),
    .i_pixel_valid(i_pixel_valid),
    .o_pixel(o_gy)
);

assign o_convolved_data = ((o_gx + o_gy) > 11'd50) ? 1'b1: 1'b0; 
endmodule