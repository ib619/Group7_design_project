module sorter (
	input [DATA_WIDTH - 1:0] pixel0,
	input [DATA_WIDTH - 1:0] pixel1, 
	input [DATA_WIDTH - 1:0] pixel2, 
	input [DATA_WIDTH - 1:0] pixel3, 
	input [DATA_WIDTH - 1:0] pixel4, 
	input [DATA_WIDTH - 1:0] pixel5, 
	input [DATA_WIDTH - 1:0] pixel6, 
	input [DATA_WIDTH - 1:0] pixel7, 
	input [DATA_WIDTH - 1:0] pixel8,
	output [DATA_WIDTH - 1:0] median_val,
	output [DATA_WIDTH - 1:0] low1_check,
	output [DATA_WIDTH - 1:0] low2_check,
	output [DATA_WIDTH - 1:0] high1_check,
	output [DATA_WIDTH - 1:0] high2_check,
	output [DATA_WIDTH - 1:0] mid1_check,
	output [DATA_WIDTH - 1:0] mid2_check
	
);
parameter DATA_WIDTH = 8;

// Compare 3 pixels
wire [DATA_WIDTH - 1:0] low0;
wire [DATA_WIDTH - 1:0] high0;
comparator c0(
	.pixel0(pixel1),
	.pixel1(pixel2), 
	.high(high0), 
	.low(low0)
);

wire [DATA_WIDTH - 1:0] low1;
wire [DATA_WIDTH - 1:0] high1;
comparator c1(
	.pixel0(pixel0),
	.pixel1(low0), 
	.high(high1), 
	.low(low1) // Lowest among the three
);

wire [DATA_WIDTH - 1:0] low2;
wire [DATA_WIDTH - 1:0] high2;
comparator c2(
	.pixel0(high1),
	.pixel1(high0), 
	.high(high2), //highest among the three
	.low(low2)
);

// Compare Next 3 pixels
wire [DATA_WIDTH - 1:0] low3;
wire [DATA_WIDTH - 1:0] high3;
comparator c3(
	.pixel0(pixel4),
	.pixel1(pixel5), 
	.high(high3), 
	.low(low3)
);

wire [DATA_WIDTH - 1:0] low4;
wire [DATA_WIDTH - 1:0] high4;
comparator c4(
	.pixel0(pixel3),
	.pixel1(low3), 
	.high(high4), 
	.low(low4)
);

wire [DATA_WIDTH - 1:0] low5;
wire [DATA_WIDTH - 1:0] high5;
comparator c5(
	.pixel0(high4),
	.pixel1(high3), 
	.high(high5), 
	.low(low5)
);

// Compare Last 3 pixels
wire [DATA_WIDTH - 1:0] low6;
wire [DATA_WIDTH - 1:0] high6;
comparator c6(
	.pixel0(pixel7),
	.pixel1(pixel8), 
	.high(high6), 
	.low(low6)
);

wire [DATA_WIDTH - 1:0] low7;
wire [DATA_WIDTH - 1:0] high7;
comparator c7(
	.pixel0(pixel6),
	.pixel1(low6), 
	.high(high7), 
	.low(low7)
);

wire [DATA_WIDTH - 1:0] low8;
wire [DATA_WIDTH - 1:0] high8;
comparator c8(
	.pixel0(high7),
	.pixel1(high6), 
	.high(high8), 
	.low(low8)
);

///////////////////////////////////////
// Comparing low values of first 3 pixels and next 3 pixels
wire [DATA_WIDTH - 1:0] high9;
comparator c9(
	.pixel0(low1),
	.pixel1(low4), 
	.high(high9),
	.low(low1_check)// low gives either the second lowest or lowest value
);
// Comparing low values with the last 3 pixels
wire [DATA_WIDTH - 1:0] high10;
comparator c10(
	.pixel0(high9),
	.pixel1(low7), 
	.high(high10),
	.low(low2_check)// low gives either the second lowest or lowest value
);

// Compare high values of last 3 pixels and next 3 pixels
wire [DATA_WIDTH - 1:0] low11;
comparator c11(
	.pixel0(high5),
	.pixel1(high8), 
	.high(high1_check),// high gives either the largest or second largest val
	.low(low11)
);

// Compare high values with first 3 pixels
wire [DATA_WIDTH - 1:0] low12;
comparator c12(
	.pixel0(high2),
	.pixel1(low11), 
	.high(high2_check),// high gives either the largest or second largest val
	.low(low12)
);


/////////////////////////////////////////////////////////
// At this point we still need to sort the middle 5 values
// Compare mid values from the 3 groups of registers
wire [DATA_WIDTH - 1:0] low13;
wire [DATA_WIDTH - 1:0] high13;
comparator c13(
	.pixel0(low8),
	.pixel1(low5), 
	.high(high13),
	.low(low13)
);


wire [DATA_WIDTH - 1:0] high14;
comparator c14(
	.pixel0(low2),
	.pixel1(low13), 
	.high(high14),
	.low(mid1_check)//3rd or 4th lowest value
);

wire [DATA_WIDTH - 1:0] low15;
comparator c15(
	.pixel0(high13),
	.pixel1(high14), 
	.high(mid2_check),// 3rd or 4th highest value
	.low(low15)
);

// Check with other 2 values
wire [DATA_WIDTH - 1:0] low16;
wire [DATA_WIDTH - 1:0] high16;
comparator c16(
	.pixel0(low12),
	.pixel1(low15), 
	.high(high16),
	.low(low16)
);
 
wire [DATA_WIDTH - 1:0] high17;
comparator c17(
	.pixel0(high10),
	.pixel1(low16), 
	.high(high17)
	// 3rd or 4th lowest value
);

comparator c18(
	.pixel0(high16),
	.pixel1(high17), 
	// 3rd or 4th highest value
	.low(median_val)
);
endmodule