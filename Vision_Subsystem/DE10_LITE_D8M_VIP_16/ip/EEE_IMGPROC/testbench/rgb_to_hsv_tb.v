module  rgb_to_hsv_tb(
    input                       clk,
    input                       rst_n,
    input	     [7:0]            rgb_r,
    input	     [7:0]            rgb_g,
    input	     [7:0]            rgb_b,	
    output reg   [8:0]            hsv_h,//  0 - 360
    output reg   [7:0]            hsv_s,// 0- 255
    output reg   [7:0]            hsv_v, // 0- 255
    output reg                    valid_out,
    input                         valid_in,

    output [13:0]  top60_check,
    output [13:0]  quotient_check,
    output [7:0]   cdiff_reg_check,
    output [13:0]  quotient_before
);
rgb_to_hsv dut(
    .clk(clk),
    .rst_n(rst_n),
    .rgb_r(rgb_r),
    .rgb_g(rgb_g),
    .rgb_b(rgb_g),	
    .hsv_h(hsv_h),
    .hsv_s(hsv_s),
    .hsv_v(hsv_v),
    .valid_in(valid_in),
    .valid_out(valid_out),
    .top60_check(top60_check),
    .quotient_check(quotient_check),
    .cdiff_reg_check(cdiff_reg_check),
    .quotient_before(quotient_before)
);


