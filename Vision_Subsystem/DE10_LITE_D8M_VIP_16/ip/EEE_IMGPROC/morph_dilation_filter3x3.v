module morph_dilation_filter3x3 (
    input clk,
    input rst_n,
    input i_pixel,
    input i_pixel_valid, // Both in_valid and packet video
    output o_convolved_data
);

wire [8:0] pre_filter;

separableconvolution3x3_dilation #(.DATA_WIDTH(1)) filter(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(i_pixel),
    .i_pixel_valid(i_pixel_valid),
    .o_pixel(o_convolved_data)
);

endmodule