
module  rgb_to_hsv(
	input	     [7:0]           	rgb_r,
	input	     [7:0]            	rgb_g,
	input	     [7:0]            	rgb_b,	
	output reg [8:0] 				  	hsv_h,//  0 - 360
	output     [7:0] 			  		hsv_s,// 0- 255
	output     [7:0] 			  		hsv_v // 0- 255
);

	reg [7:0] top;//molecular
	reg [13:0] top_60;//*60
	reg [2:0] rgb_se;
	reg [2:0] rgb_se_n;//direction
	wire [7:0] cmax;//Maximum weight
	wire [7:0] cmin;//Minimum component
	wire [7:0] cdiff;//max - min
	wire r_more_g;
	wire r_more_b;
	wire g_more_b;
	reg [7:0] division;//division
	
	
	assign r_more_g = (rgb_r > rgb_g)? 1'b1:1'b0; 
	assign r_more_b = (rgb_r > rgb_b)? 1'b1:1'b0; 
	assign g_more_b = (rgb_g > rgb_b)? 1'b1:1'b0; 
	
	assign cmax = (r_more_g & r_more_b) ? rgb_r : (g_more_b & ~r_more_g) ? rgb_g : rgb_b; 
	assign cmin = (~r_more_g & ~r_more_b) ? rgb_r : (g_more_b & r_more_b) ? rgb_b : rgb_g;
	assign cdiff = cmax - cmin;
	
	assign hsv_s = (cmax > 8'd0)? {cdiff[7:0],8'b00000000} / cmax : 8'd0;
	assign hsv_v = cmax;
	
	always @(*)
		begin
			// Max = R
			if (r_more_g & r_more_b)
				begin
					top = (g_more_b) ? rgb_g - rgb_b : rgb_b - rgb_g;
					top_60 = $signed(top) * 6'd60;
					division = (cdiff > 8'd0) ? $signed(top_60) / cdiff : 8'd240;
					hsv_h = (rgb_g >= rgb_b) ? division : 9'd360 - division;
				
				end
			// Max = G
			else if (g_more_b &~r_more_g) 
				begin
					top = (r_more_b) ? rgb_r - rgb_b : rgb_b - rgb_r;
					top_60 = $signed(top) * 6'd60;
					division = (cdiff > 8'd0) ? $signed(top_60) / cdiff : 8'd240;
					hsv_h = (r_more_b) ? 9'd120 - division  :division + 9'd120;
				end

			// Max = B
			else 
				begin
					top = (r_more_g) ? rgb_r - rgb_g : rgb_g - rgb_r;
					top_60 = $signed(top) * 6'd60;
					division = (cdiff > 8'd0) ? $signed(top_60) / cdiff : 8'd240;
					hsv_h = (r_more_g) ? division + 9'd240 : 9'd240 - division ;
				end
		end

endmodule


   