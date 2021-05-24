module colour_threshold(
    input     clk,
    input     rst_n,
    input     [8:0] 	hue,//  0 - 360
    input     [7:0]  saturation,// 0- 255
    input     [7:0] 	value_b, // 0- 255
    input     valid_in,
	 input     [21:0]  pixel_addr,
    output     red_detect,
    output     green_detect,
    output     blue_detect,
    output     grey_detect,
    output     yellow_detect,
	 output    [21:0] pixel_addr_out
);

wire pf_red_detect, pf_blue_detect, pf_green_detect, pf_grey_detect, pf_yellow_detect;

assign pf_red_detect = (hue < 8'd30) & (hue > 8'd0)
                        & (saturation < 8'd255) & ( saturation > 8'd90) 
                        & (value_b < 8'd255 ) & ( value_b > 8'd90);


assign pf_green_detect = (hue < 8'd135) & (hue > 8'd110)
                        & (saturation < 8'd180) & ( saturation > 8'd100) 
                        & (value_b < 8'd180 ) & ( value_b > 8'd100);
                            

assign pf_blue_detect = (hue < 8'd220) & (hue > 8'd150)
                        & (saturation < 8'd255) & ( saturation > 8'd50) 
                        & (value_b < 8'd255 ) & ( value_b > 8'd75);


assign pf_grey_detect = (hue < 8'd180) & (hue > 8'd0)
                        & (saturation < 8'd32) & ( saturation > 8'd0) 
                        & (value_b < 8'd70) & ( value_b > 8'd10);
                        
assign pf_yellow_detect = (hue < 8'd50) & (hue > 8'd36)
                        & (saturation < 8'd255) & ( saturation > 8'd150) 
                        & (value_b < 8'd255 ) & ( value_b > 8'd80);
                            
///////////////////////////////////////////////////////////////////////
//Morph Filter
morph_filter ero_r(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_red_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
	 .i_pixel_addr(pixel_addr),
    .o_convolved_data(red_detect),
	 .o_pixel_addr_out(pixel_addr_out)
);

morph_filter ero_g(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_green_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(green_detect)
);

morph_filter ero_b(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_blue_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(blue_detect)
);

morph_filter ero_gr(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_grey_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(grey_detect)
);

morph_filter ero_y(
    .clk(clk),
    .rst_n(rst_n),
    .i_pixel(pf_yellow_detect),
    .i_pixel_valid(valid_in), // Both in_valid and packet video
    .o_convolved_data(yellow_detect)
);
					
endmodule