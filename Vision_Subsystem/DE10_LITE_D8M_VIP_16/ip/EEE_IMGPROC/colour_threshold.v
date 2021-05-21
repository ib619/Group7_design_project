module colour_threshold(

	input     [8:0] 	hue,//  0 - 360
	input     [7:0]   saturation,// 0- 255
	input     [7:0] 	value_b, // 0- 255
	output     red_detect,
	output	  green_detect,
	output	  blue_detect,
	output     grey_detect,
	output     yellow_detect
);
	// Detect red areas

//	assign red_detect = (hue < 8'd20) & (hue > 8'd0)
//							  & (saturation < 8'd255) & ( saturation > 8'd60) 
//							  & (value_b < 8'd255 ) & ( value_b > 8'd60);

	assign red_detect = (hue < 8'd30) & (hue > 8'd0)
							  & (saturation < 8'd255) & ( saturation > 8'd90) 
							  & (value_b < 8'd255 ) & ( value_b > 8'd90);


//	assign green_detect = (hue < 8'd130) & (hue > 8'd100)
//								 & (saturation < 8'd255) & ( saturation > 8'd115) 
//								 & (value_b < 8'd255 ) & ( value_b > 8'd75);
	assign green_detect = (hue < 8'd135) & (hue > 8'd110)
								 & (saturation < 8'd180) & ( saturation > 8'd100) 
								 & (value_b < 8'd180 ) & ( value_b > 8'd100);
								 

//	assign blue_detect = (hue < 8'd220) & (hue > 8'd160)
//							  & (saturation < 8'd255) & ( saturation > 8'd50) 
//							  & (value_b < 8'd255 ) & ( value_b > 8'd75);
	assign blue_detect = (hue < 8'd220) & (hue > 8'd150)
							  & (saturation < 8'd255) & ( saturation > 8'd50) 
							  & (value_b < 8'd255 ) & ( value_b > 8'd75);

//	assign grey_detect = (hue < 8'd100) & (hue > 8'd40)
//							  & (saturation < 8'd150) & ( saturation > 8'd10) 
//							  & (value_b < 8'd50 ) & ( value_b > 8'd20);
	assign grey_detect = (hue < 8'd140) & (hue > 8'd60)
							  & (saturation < 8'd127) & ( saturation > 8'd50) 
							  & (value_b < 8'd90) & ( value_b > 8'd50);
							  
//	assign yellow_detect = (hue < 8'd60) & (hue > 8'd20)
//							  & (saturation < 8'd255) & ( saturation > 8'd100) 
//							  & (value_b < 8'd255 ) & ( value_b > 8'd80);
	assign yellow_detect = (hue < 8'd50) & (hue > 8'd36)
							  & (saturation < 8'd255) & ( saturation > 8'd150) 
							  & (value_b < 8'd255 ) & ( value_b > 8'd80);
endmodule