module perspective_transform(
	input clk,
	input rst_n,
	input [10:0] x_coord,
	input [10:0] y_coord,
	output [10:0] x_coord_adj,
	output [10:0] y_coord_adj,
	output [27:0] x_coord_temp_check,
	output [27:0] y_coord_temp_check,
	output [27:0] x_coord_temp2_check,
	output [27:0] y_coord_temp2_check,
	output [55:0] M1_temp_check,
	output [55:0] M2_temp_check,
	output [55:0] M4_temp_check,
	output [55:0] M5_temp_check,
	output [55:0] M7_temp_check,
	output [55:0] M8_temp_check,
	output [27:0] w_check
);
localparam M1 = 28'hd3dc;//6645
localparam M2 = 28'h13dcb;//7160
localparam M3 = 28'h253dcb0;//3437202


localparam M4 = 28'h0;
localparam M5 = 28'h234f7;//13748
localparam M6 = 28'h29611a7;//3409705

localparam M7 = 28'h0;
localparam M8 = 28'hf9;//22
localparam M9 = 28'h10000;//4096

wire [27:0] x_coord_temp;
wire [27:0] y_coord_temp;
wire [27:0] w;

wire [27:0] x_coord_temp2;
wire [27:0] y_coord_temp2;

assign x_coord_temp_check = x_coord_temp;
assign y_coord_temp_check = y_coord_temp;
assign x_coord_temp2_check = x_coord_temp2;
assign y_coord_temp2_check = y_coord_temp2;

reg [55:0] M1_temp;
reg [55:0] M2_temp;
reg [55:0] M4_temp;
reg [55:0] M5_temp;
reg [55:0] M7_temp;
reg [55:0] M8_temp;
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
			M1_temp <= 55'd0;
			M2_temp <= 55'd0;
			M4_temp <= 55'd0;
			M5_temp <= 55'd0;
			M7_temp <= 55'd0;		
			M8_temp <= 55'd0;
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