module perspective_transform(
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

localparam M1 = 24'h19eb;
localparam M2 = 24'h1bd7;
localparam M3 = 24'h34728f;


localparam M4 = 24'h0;
localparam M5 = 24'h3599;
localparam M6 = 24'h340733;

localparam M7 = 24'h0;
localparam M8 = 24'h16;
localparam M9 = 24'h1000;

assign x_coord_temp_check = x_coord_temp;
assign y_coord_temp_check = y_coord_temp;
assign x_coord_temp2_check = x_coord_temp2;
assign y_coord_temp2_check = y_coord_temp2;

wire [47:0] M1_temp = M1 * x_coord;
wire [47:0] M2_temp = M2 * y_coord;
wire [47:0] M4_temp = M4 * x_coord;
wire [47:0] M5_temp = M5 * y_coord;
wire [47:0] M7_temp = M7 * x_coord;
wire [47:0] M8_temp = M8 * y_coord;


assign M1_temp_check = M1_temp;
assign M2_temp_check = M2_temp;
assign M4_temp_check = M4_temp;
assign M5_temp_check = M5_temp;
assign M7_temp_check = M7_temp;
assign M8_temp_check = M8_temp;
assign w_check = w;
wire [23:0] x_coord_temp;
wire [23:0] y_coord_temp;
wire [23:0] w;

wire [23:0] x_coord_temp2;
wire [23:0] y_coord_temp2;

assign x_coord_temp = M3 -M1_temp - M2_temp ;
assign y_coord_temp = M4_temp - M5_temp + M6;

assign w =  M7_temp - M8_temp + M9;

signed_divide dx(
	.denom(w),
	.numer(x_coord_temp),
	.quotient(x_coord_temp2)
);

signed_divide dy(
	.denom(w),
	.numer(y_coord_temp),
	.quotient(y_coord_temp2)
);

assign x_coord_adj = x_coord_temp2[10:0];
assign y_coord_adj = y_coord_temp2[10:0];
endmodule