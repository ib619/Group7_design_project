module colour_threshold(
    input     clk,
    input     rst_n,
    input     [8:0] 	hue,//  0 - 360
    input     [7:0]  saturation,// 0- 255
    input     [7:0] 	value_b, // 0- 255
    input     valid_in,
//	 input     [21:0]  pixel_addr,
	 input      erosion_mode,
	 input      dilation_mode,
    output     red_detect,
    output     green_detect,
    output     blue_detect,
    output     grey_detect,
    output     yellow_detect,
	 output     red_edge_detect,
	 output     blue_edge_detect,
	 output     green_edge_detect,
	 output     grey_edge_detect,
	 output     yellow_edge_detect
//	 output    [21:0] pixel_addr_out
);

wire pf_red_detect, pf_blue_detect, pf_green_detect, pf_grey_detect, pf_yellow_detect;
wire erosion_red_detect, erosion_blue_detect, erosion_green_detect, erosion_grey_detect, erosion_yellow_detect;
wire dilation_red_detect, dilation_blue_detect, dilation_green_detect, dilation_grey_detect, dilation_yellow_detect;

assign red_detect = (dilation_mode) ? dilation_red_detect: (erosion_mode) ? erosion_red_detect: pf_red_detect;
assign blue_detect = (dilation_mode) ? dilation_blue_detect: (erosion_mode) ? erosion_blue_detect: pf_blue_detect;
assign green_detect = (dilation_mode) ? dilation_green_detect: (erosion_mode) ? erosion_green_detect: pf_green_detect;
assign grey_detect = (dilation_mode) ? dilation_grey_detect: (erosion_mode) ? erosion_grey_detect: pf_grey_detect;
assign yellow_detect = (dilation_mode) ? dilation_yellow_detect: (erosion_mode) ? erosion_yellow_detect: pf_yellow_detect;


assign pf_red_detect = (hue < 8'd30) & (hue > 8'd0)
                        & (saturation < 8'd255) & ( saturation > 8'd110) 
                        & (value_b < 8'd255 ) & ( value_b > 8'd30);


assign pf_green_detect = (hue < 8'd135) & (hue > 8'd110)
                        & (saturation < 8'd180) & ( saturation > 8'd100) 
                        & (value_b < 8'd180 ) & ( value_b > 8'd80);
                            

assign pf_blue_detect = (hue < 8'd220) & (hue > 8'd150)
                        & (saturation < 8'd255) & ( saturation > 8'd70) 
                        & (value_b < 8'd255 ) & ( value_b > 8'd60);


assign pf_grey_detect = ((hue < 8'd90) & (hue > 8'd40)
                        & (saturation < 8'd40) & ( saturation > 8'd0) 
                        & (value_b < 8'd65) & ( value_b > 8'd0)) | 
								((hue < 8'd220) & (hue > 8'd140)
                        & (saturation < 8'd70) & ( saturation > 8'd0) 
                        & (value_b < 8'd60) & ( value_b > 8'd30));
                        
assign pf_yellow_detect = (hue < 8'd50) & (hue > 8'd30)
                        & (saturation < 8'd255) & ( saturation > 8'd120) 
                        & (value_b < 8'd255 ) & ( value_b > 8'd60);


//assign pf_red_detect = (hue < 8'd40) & (hue > 8'd0)
//                        & (saturation < 8'd220) & ( saturation > 8'd110) 
//                        & (value_b < 8'd255 ) & ( value_b > 8'd120);
//
//assign pf_green_detect = (hue < 8'd140) & (hue > 8'd70)
//                        & (saturation < 8'd230) & ( saturation > 8'd130) 
//                        & (value_b < 8'd200 ) & ( value_b > 8'd60);
//                            
//
//assign pf_blue_detect = (hue < 8'd200) & (hue > 8'd100)
//                        & (saturation < 8'd135) & ( saturation > 8'd55) 
//                        & (value_b < 8'd200 ) & ( value_b > 8'd50);
//
//
//assign pf_grey_detect = (hue < 8'd100) & (hue > 8'd0)
//                        & (saturation < 8'd130) & ( saturation > 8'd20) 
//                        & (value_b < 8'd100) & ( value_b > 8'd20);
//                        
//
//assign pf_yellow_detect = (hue < 8'd80) & (hue > 8'd20)
//                        & (saturation < 8'd255) & ( saturation > 8'd120) 
//                        & (value_b < 8'd200 ) & ( value_b > 8'd100);

///////////////////////////////////////////////////////////////////////
//Morph Erosion Filter
morph_erosion_filter3x3 ero_r(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_red_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(erosion_red_detect)
);

morph_erosion_filter3x3 ero_g(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_green_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(erosion_green_detect)
);

morph_erosion_filter3x3 ero_b(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_blue_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(erosion_blue_detect)
);

morph_erosion_filter3x3 ero_gr(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_grey_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(erosion_grey_detect)
);

morph_erosion_filter3x3 ero_y(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_yellow_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(erosion_yellow_detect)
);
//Morph Dilation Filter
morph_dilation_filter3x3 dil_r(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(erosion_red_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(dilation_red_detect)
);

morph_dilation_filter3x3 dil_g(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(erosion_green_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(dilation_green_detect)
);

morph_dilation_filter3x3 dil_b(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(erosion_blue_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(dilation_blue_detect)
);

morph_dilation_filter3x3 dil_gr(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(erosion_grey_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(dilation_grey_detect)
);

morph_dilation_filter3x3 dil_y(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(erosion_yellow_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(dilation_yellow_detect)
);

// Edge Detection
wire [7:0] dilation_red_detect_8 = (dilation_red_detect) ? 8'd255 : 8'd0;
wire [7:0] dilation_green_detect_8 = (dilation_green_detect) ? 8'd255 : 8'd0;
wire [7:0] dilation_blue_detect_8 = (dilation_blue_detect) ? 8'd255 : 8'd0;
wire [7:0] dilation_grey_detect_8 = (dilation_grey_detect) ? 8'd255 : 8'd0;
wire [7:0] dilation_yellow_detect_8 = (dilation_yellow_detect) ? 8'd255 : 8'd0;

//sobel_filter3x3 sobelfilter_red (
//	.clk(clk),
//	.rst_n(rst_n),
//	.i_pixel(dilation_red_detect_8),
//	.i_pixel_valid(valid_in), // Both in_valid and packet video
//	.o_convolved_data(red_edge_detect)
//);
//sobel_filter3x3 sobelfilter_green (
//	.clk(clk),
//	.rst_n(rst_n),
//	.i_pixel(dilation_green_detect_8),
//	.i_pixel_valid(valid_in), // Both in_valid and packet video
//	.o_convolved_data(green_edge_detect)
//);
//sobel_filter3x3 sobelfilter_blue (
//	.clk(clk),
//	.rst_n(rst_n),
//	.i_pixel(dilation_blue_detect_8),
//	.i_pixel_valid(valid_in), // Both in_valid and packet video
//	.o_convolved_data(blue_edge_detect)
//);
sobel_filter3x3 sobelfilter_grey (
	.clk(clk),
	.rst_n(rst_n),
	.i_pixel(dilation_grey_detect_8),
	.i_pixel_valid(valid_in), // Both in_valid and packet video
	.o_convolved_data(grey_edge_detect)
);
//sobel_filter3x3 sobelfilter_yellow (
//	.clk(clk),
//	.rst_n(rst_n),
//	.i_pixel(dilation_yellow_detect_8),
//	.i_pixel_valid(valid_in), // Both in_valid and packet video
//	.o_convolved_data(yellow_edge_detect)
//);					
endmodule