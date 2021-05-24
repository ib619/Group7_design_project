module morph_filter (
    input clk,
    input rst_n,
    input i_pixel,
    input i_pixel_valid, // Both in_valid and packet video
    input [21:0]  i_pixel_addr,
    output o_convolved_data,
    output [21:0] o_pixel_addr_out
);

wire [8:0] pre_filter;


control control1(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(i_pixel),
    .i_pixel_valid(i_pixel_valid),
    .i_pixel_addr(i_pixel_addr),
    .o_pixel(pre_filter),
    .o_pixel_addr_out(o_pixel_addr_out)
);

erosion erosion1(
    .i_pixel(pre_filter),
    .o_convolved_data(o_convolved_data)
);

endmodule