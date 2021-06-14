`timescale 1 ns/10 ps  // time-unit = 1 ns, precision = 10 ps


module gaussian_filter_tb(
);
    reg clk;
    reg rst_n;
    reg [7:0] i_pixel;
    wire [7:0] o_convolved_data;
	 wire [7:0] tap0;
	 wire [7:0] tap1;
	 wire [7:0] tap2;
	 wire [7:0] tap3;
	 wire pixelEnable_check;
 	 wire [18:0] pixelCounter1;
 	 wire [9:0] pixelRowCounter1;
	 wire [11:0] pixelStartUpCounter_check;
    reg i_valid;
	 wire o_valid;
    localparam period = 20; 
	 integer i;

    // Clock Generation
    always 
    begin
        clk = 1'b1; 
        #20; // high for 20 * timescale = 20 ns

        clk = 1'b0;
        #20; // low for 20 * timescale = 20 ns
    end

    // Testing
    initial begin
	 	  rst_n = 1'b0;
        i_valid = 1'b0;
		  
		  @(posedge clk)
		  rst_n = 1'b1;
        i_valid = 1'b1;
		  #1;
		  i_pixel = 1;
        

		  for (i = 2; i < 55; i=i+1)
		  begin
		      @(posedge clk)
				#1;
				i_pixel = i;
				#2;
            $display("Convolved Data Output=%d", o_convolved_data);

		  end
		  
		  @(posedge clk)
        i_pixel = 0;
		  
		  for (i = 0; i < 55; i=i+1)
		  begin
		      @(posedge clk)
				#2;
            $display("Convolved Data Output=%d", o_convolved_data);

		  end
        $finish;
    end

    separableconvolution5x5_gaussian #(.DATA_WIDTH(8)) dut(
        .clk(clk),
        .rst_n(rst_n),
        .i_pixel(i_pixel),
        .i_pixel_valid(i_valid),
        .o_pixel(o_convolved_data),
		  .tap0(tap0),
		  .tap1(tap1),
		  .tap2(tap2),
		  .tap3(tap3),
		  .pixelEnable_check(pixelEnable_check),
 	     .pixelCounter1(pixelCounter1),
 	     .pixelRowCounter1(pixelRowCounter1),
	     .pixelStartUpCounter_check(pixelStartUpCounter_check)
    );

endmodule