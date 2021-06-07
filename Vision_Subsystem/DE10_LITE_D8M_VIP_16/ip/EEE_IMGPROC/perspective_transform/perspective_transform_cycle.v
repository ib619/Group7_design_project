module perspective_transform_cycle(
	input clk,
	input rst_n,
	input [10:0] x_coord,
	input [10:0] y_coord,
	output [10:0] x_coord_adj,
	output [10:0] y_coord_adj,
	output [23:0] x_coord_temp_check,
	output [23:0] y_coord_temp_check,
	output [23:0] x_coord_temp2_check,
	output [23:0] y_coord_temp2_check,
	output [47:0] M1_temp_check,
	output [47:0] M2_temp_check,
	output [47:0] M4_temp_check,
	output [47:0] M5_temp_check,
	output [47:0] M7_temp_check,
	output [47:0] M8_temp_check,
	output [23:0] w_check
);

localparam M1 = 24'h19f5;//6645
localparam M2 = 24'h1bf8;//7160
localparam M3 = 24'h347292;//3437202


localparam M4 = 24'h0;
localparam M5 = 24'h35b4;//13748
localparam M6 = 24'h340729;//3409705

localparam M7 = 24'h0;
localparam M8 = 24'h16;//22
localparam M9 = 24'h1000;//4096

assign x_coord_temp_check = x_coord_temp;
assign y_coord_temp_check = y_coord_temp;
assign x_coord_temp2_check = x_coord_temp2;
assign y_coord_temp2_check = y_coord_temp2;

reg [47:0] M1_temp;
reg [47:0] M2_temp;
reg [47:0] M4_temp;
reg [47:0] M5_temp;
reg [47:0] M7_temp;
reg [47:0] M8_temp;
assign M1_temp_check = M1_temp;
assign M2_temp_check = M2_temp;
assign M4_temp_check = M4_temp;
assign M5_temp_check = M5_temp;
assign M7_temp_check = M7_temp;
assign M8_temp_check = M8_temp;

// First Cycle
always @(posedge clk)
	begin
		if (~rst_n) begin
			M1_temp <= 48'd0;
			M2_temp <= 48'd0;
			M4_temp <= 48'd0;
			M5_temp <= 48'd0;
			M7_temp <= 48'd0;		
			M8_temp <= 48'd0;
		end
		else begin
			M1_temp <= M1 * x_coord;
			M2_temp <= M2 * y_coord;
			M4_temp <= M4 * x_coord;
			M5_temp <= M5 * y_coord;
			M7_temp <= M7 * x_coord;		
			M8_temp <= M8 * y_coord;
		end
	end
// Second Cycle

assign w_check = w;
wire [23:0] x_coord_temp;
wire [23:0] y_coord_temp;
wire [23:0] w;

wire [23:0] x_coord_temp2;
wire [23:0] y_coord_temp2;

assign x_coord_temp = M3 -M1_temp - M2_temp ;
assign y_coord_temp = M4_temp - M5_temp + M6;

assign w =  M7_temp - M8_temp + M9;

signed_divide_delay dx(
	.denom(w),
	.numer(x_coord_temp),
	.quotient(x_coord_temp2),
	.aclr(~rst_n),
	.clock(clk)
);

signed_divide_delay dy(
	.denom(w),
	.numer(y_coord_temp),
	.quotient(y_coord_temp2),
	.aclr(~rst_n),
	.clock(clk)
);

assign x_coord_adj = x_coord_temp2[10:0];
assign y_coord_adj = y_coord_temp2[10:0];
endmodule