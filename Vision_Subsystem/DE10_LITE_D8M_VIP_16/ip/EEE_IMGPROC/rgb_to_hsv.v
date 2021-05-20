module  rgb_to_hsv(
	input 								clk,	
	input                         rst_n,
	input	     [7:0]           	rgb_r,
	input	     [7:0]            	rgb_g,
	input	     [7:0]            	rgb_b,	
	output reg [8:0] 				  	hsv_h,//  0 - 360
	output reg [7:0] 			  		hsv_s,// 0- 255
	output reg [7:0] 			  		hsv_v, // 0- 255
	output [7:0] cmax_reg_w, 
	output [7:0] cdiff_reg_w, 
	output [13:0] top_w, 
	output r_more_g_reg_w,
	output r_more_b_reg_w,
	output g_more_b_reg_w,
	output [7:0] div
);


    reg [7:0] top;//numerator

    wire [7:0] cmax;//Maximum weight
    wire [7:0] cmin;//Minimum component
    wire [7:0] cdiff;//max - min

    wire r_more_g;
    wire r_more_b;
    wire g_more_b;

    reg [7:0] division;//division
    reg [8:0] hsv_h_interm;
    wire [7:0] hsv_s_interm;


    // Pipeline Registers
    reg [7:0] cmax_reg; 
    reg [7:0] cdiff_reg;

    reg r_more_g_reg1;
    reg r_more_b_reg1;
    reg g_more_b_reg1;
    reg r_more_g_reg2;
    reg r_more_b_reg2;
    reg g_more_b_reg2;
    reg [13:0] top_60;//*60
    reg [7:0] division_reg;//division
    reg [8:0] hsv_s_interm2;
    reg [8:0] hsv_v_interm;

    //testing

    assign cmax_reg_w= cmax_reg;
    assign cdiff_reg_w= cdiff_reg; 
    assign top_w = top_60; 
    assign r_more_g_reg_w= r_more_g_reg1;
    assign r_more_b_reg_w = r_more_b_reg1;
    assign g_more_b_reg_w = g_more_b_reg1;
    assign div = division;

    assign r_more_g = (rgb_r > rgb_g)? 1'b1:1'b0; 
    assign r_more_b = (rgb_r > rgb_b)? 1'b1:1'b0; 
    assign g_more_b = (rgb_g > rgb_b)? 1'b1:1'b0; 

    assign cmax = (r_more_g & r_more_b) ? rgb_r : (g_more_b & ~r_more_g) ? rgb_g : rgb_b; 
    assign cmin = (~r_more_g & ~r_more_b) ? rgb_r : (g_more_b & r_more_b) ? rgb_b : rgb_g;
    assign cdiff = cmax - cmin;


    always @(*)
        begin
            // Max = R
            if (r_more_g & r_more_b)
                begin
                    top = (g_more_b) ? rgb_g - rgb_b : rgb_b - rgb_g;
                end
            // Max = G
            else if (g_more_b &~r_more_g) 
                begin
                    top = (r_more_b) ? rgb_r - rgb_b : rgb_b - rgb_r;
                end
            // Max = B
            else 
                begin
                    top = (r_more_g) ? rgb_r - rgb_g : rgb_g - rgb_r;
                end
        end

    always@(posedge clk) 
        begin
            if (~rst_n) begin
                cmax_reg <= 8'd0;
                cdiff_reg <= 8'd0;
                r_more_g_reg1 <= 1'b0;
                r_more_b_reg1 <= 1'b0;
                g_more_b_reg1 <= 1'b0;
                r_more_g_reg2 <= 1'b0;
                r_more_b_reg2 <= 1'b0;
                g_more_b_reg2 <= 1'b0;
                top_60 <= 14'd0;
                end
            else begin
                cmax_reg <= cmax;
                cdiff_reg <= cdiff;
                r_more_g_reg1 <= r_more_g;
                r_more_b_reg1 <= r_more_b;
                g_more_b_reg1 <= g_more_b;
                r_more_g_reg2 <= r_more_g_reg1;
                r_more_b_reg2 <= r_more_b_reg1;
                g_more_b_reg2 <= g_more_b_reg1;
                top_60 <= {top,6'b000000} - {top,2'b00};
                end
            end
            
    always @(*)
        begin
            // Max = R
            if (r_more_g_reg1 & r_more_b)
                begin
                    division = (cdiff_reg == 8'd0) ?  8'd240: $signed(top_60) / cdiff_reg;
                end
            // Max = G
            else if (g_more_b_reg1 & ~r_more_g_reg1) 
                begin
                    division = (cdiff_reg == 8'd0) ?  8'd240:$signed(top_60) / cdiff_reg;
                end
            // Max = B
            else 
                begin
                    division = (cdiff_reg == 8'd0) ? 8'd240: $signed(top_60) / cdiff_reg;
                end
        end
    assign hsv_s_interm =  (cmax_reg == 8'd0)? 8'd0: {cdiff_reg[7:0],8'b00000000} / cmax_reg ;
    always@(posedge clk) 
        begin
            if (~rst_n) begin
                division_reg <= 8'd0;
                hsv_s_interm2 <= 8'd0;
                hsv_v_interm <= 8'd0;
                end
            else begin
                division_reg <= division;	
                hsv_s_interm2 <= hsv_s_interm;
                hsv_v_interm <= cmax_reg;	
            end
        end
            
    always @(*)
    begin
        // Max = R
        if (r_more_g_reg2 & r_more_b_reg2)
            begin
                hsv_h_interm = (g_more_b_reg2) ? division_reg : 9'd360 - division_reg;
            
            end
        // Max = G
        else if (g_more_b_reg2 & ~r_more_g_reg2) 
            begin
                hsv_h_interm = (r_more_b_reg2) ? 9'd120 - division_reg  :division_reg + 9'd120;
            end
        // Max = B
        else 
            begin
                hsv_h_interm = (r_more_g_reg2) ? division_reg + 9'd240 : 9'd240 - division_reg ;
                
            end
    end

    always@(posedge clk) 
        begin
            if (~rst_n) begin
                hsv_h <= 9'd0;
                hsv_s <= 8'd0;
                hsv_v <= 8'd0;
            end
            else begin
                hsv_h <= hsv_h_interm;	
                hsv_s <= hsv_s_interm2;
                hsv_v <= hsv_v_interm;
            end
        end
            
endmodule