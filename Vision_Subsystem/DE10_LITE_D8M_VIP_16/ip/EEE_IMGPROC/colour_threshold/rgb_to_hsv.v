module  rgb_to_hsv(
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
    //Testing
    output [13:0]  top60_check,
    output [13:0]  quotient_check,
    output [7:0]   cdiff_reg_check
);


reg [7:0] top;//numerator

wire [7:0] cmax;//Maximum weight
wire [7:0] cmin;//Minimum component
wire [7:0] cdiff;//max - min

wire r_more_g;
wire r_more_b;
wire g_more_b;
wire b_more_g;

reg [7:0] division;//division
reg [8:0] hsv_h_interm;
wire [7:0] hsv_s_interm;
wire [15:0] quotient_s;
wire [13:0] quotient_h;

// Pipeline Registers
reg [7:0] cmax_reg; 
reg [7:0] cmax_reg1; 
reg [7:0] cdiff_reg;
reg [7:0] cdiff_reg1;

reg r_more_g_reg1;
reg r_more_b_reg1;
reg g_more_b_reg1;
reg b_more_g_reg1;

reg r_more_g_reg2;
reg r_more_b_reg2;
reg g_more_b_reg2;
reg b_more_g_reg2;

reg [13:0] top_60;//*60
reg [7:0] division_reg;//division
reg [8:0] hsv_s_interm2;
reg [8:0] hsv_v_interm;


reg valid1;
reg valid2;

//Testing
assign top60_check = top_60;
assign cdiff_reg_check = cdiff_reg;
assign quotient_check = quotient_s;


assign r_more_g = (rgb_r > rgb_g)? 1'b1:1'b0; 
assign r_more_b = (rgb_r > rgb_b)? 1'b1:1'b0; 
assign g_more_b = (rgb_g > rgb_b)? 1'b1:1'b0; 
assign b_more_g = (rgb_b > rgb_g)? 1'b1:1'b0;

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
        else if (b_more_g)
            begin
                top = (r_more_g) ? rgb_r - rgb_g : rgb_g - rgb_r;
            end
        else top = 8'b0;
    end

always@(posedge clk) 
    begin
        if (~rst_n) begin
            cmax_reg <= 8'd0;
            cmax_reg1 <= 8'd0;
            cdiff_reg <= 8'd0;
            cdiff_reg1 <= 8'd0;
            r_more_g_reg1 <= 1'b0;
            r_more_b_reg1 <= 1'b0;
            g_more_b_reg1 <= 1'b0;
            b_more_g_reg1 <= 1'b0;

            r_more_g_reg2 <= 1'b0;
            r_more_b_reg2 <= 1'b0;
            g_more_b_reg2 <= 1'b0;
            b_more_g_reg2 <= 1'b0;
            top_60 <= 14'd0;
                
            valid1 <= 1'b0;
            end
        else begin
            cmax_reg <= cmax;
            cmax_reg1 <= cmax_reg;
            cdiff_reg <= cdiff;
            cdiff_reg1 <= cdiff_reg;
            r_more_g_reg1 <= r_more_g;
            r_more_b_reg1 <= r_more_b;
            g_more_b_reg1 <= g_more_b;
            b_more_g_reg1 <= b_more_g;
            

            r_more_g_reg2 <= r_more_g_reg1;
            r_more_b_reg2 <= r_more_b_reg1;
            g_more_b_reg2 <= g_more_b_reg1;
            b_more_g_reg2 <= b_more_g_reg1;
            top_60 <= {top,6'b000000} - {top,2'b00};
                
            valid1 <= valid_in;
                
        end
    end


divider1	divide_h (
	.aclr ( ~rst_n ),
	.clken ( valid1 ),
	.clock ( clk ),
	.denom ( cdiff_reg ),
	.numer ( top_60 ),
	.quotient ( quotient_h )
	);

	
always @(*)
    begin
        // Max = R
        if (r_more_g_reg1 & r_more_b_reg1)
            begin
                division = (cdiff_reg1 == 8'd0) ?  8'd240: quotient_h;
            end
        // Max = G
        else if (g_more_b_reg1 & ~r_more_g_reg1) 
            begin
                division = (cdiff_reg1 == 8'd0) ?  8'd240: quotient_h;
            end
        // Max = B
        else if (b_more_g_reg1 ) 
            begin
                division = (cdiff_reg1 == 8'd0) ? 8'd240: quotient_h;
            end
        else division = 8'b0;
    end

divider2	divide_s (
	.aclr ( ~rst_n ),
	.clken ( valid1 ),
	.clock ( clk ),
	.denom ( cmax_reg ),
	.numer ( {cdiff_reg[7:0],8'b00000000} ),
	.quotient ( quotient_s )
);

assign hsv_s_interm =  (cmax_reg1 == 8'd0)? 8'd0: quotient_s ;

always@(posedge clk) 
    begin
        if (~rst_n) begin
            hsv_v_interm <= 8'd0;
            valid2 <= 1'b0;
            end
        else begin
            hsv_v_interm <= cmax_reg;	
            valid2 <= valid1;
        end
    end
		
always @(*)
    begin
        // Max = R
        if (r_more_g_reg2 & r_more_b_reg2)
            begin
                hsv_h_interm = (g_more_b_reg2) ? division : 9'd360 - division;
            end
        // Max = G
        else if (g_more_b_reg2 & ~r_more_g_reg2) 
            begin
                hsv_h_interm = (r_more_b_reg2) ? 9'd120 - division  :division + 9'd120;
            end
        // Max = B
        else if (b_more_g_reg2)
            begin
                hsv_h_interm = (r_more_g_reg2) ? division + 9'd240 : 9'd240 - division ;

            end
        else hsv_h_interm = 9'd0;
    end

always@(posedge clk) 
    begin
        if (~rst_n) begin
            hsv_h <= 9'd0;
            hsv_s <= 8'd0;
            hsv_v <= 8'd0;
            valid_out <= 1'b0;
        end
        else begin
            hsv_h <= hsv_h_interm;	
            hsv_s <= hsv_s_interm;
            hsv_v <= hsv_v_interm;
            valid_out <= valid2;
        end
    end
                
endmodule