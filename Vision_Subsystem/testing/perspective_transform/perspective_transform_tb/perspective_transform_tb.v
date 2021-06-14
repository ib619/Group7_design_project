`timescale 1 ns/10 ps  // time-unit = 1 ns, precision = 10 ps


module perspective_transform_tb(
);
    reg clk;
    reg rst_n;
    reg[10:0] x_coord, y_coord;
	 wire[10:0] x_coord_adj, y_coord_adj;
    localparam period = 20; 

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
		  
		  // Test the edges
        @(posedge clk)
        #1;
        rst_n=1'b1;
        x_coord = 11'd280;
        y_coord = 11'd310;

        @(posedge clk)
        #1;
        x_coord = 11'd370;
        y_coord = 11'd310;

        $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);
        
        @(posedge clk)
        #1;
        x_coord = 11'd20;
        y_coord = 11'd470;

        $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);

        @(posedge clk)
        #1;
        x_coord = 11'd630;
        y_coord = 11'd470;

        $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);
        
		  // Test the centre
        @(posedge clk)
        #1;
		  x_coord = 11'd260;
        y_coord = 11'd360;
        
		  $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);

        @(posedge clk)
        #1;
		  x_coord = 11'd390;
        y_coord = 11'd360;
		  
        $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);
        
        @(posedge clk)
        #1;
		  x_coord = 11'd100;
        y_coord = 11'd420;
		  
        $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);
                
        @(posedge clk)
        #1;
		  x_coord = 11'd500;
        y_coord = 11'd420;
		  
        $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);
        @(posedge clk)
        #1;
		  
        $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);        
        @(posedge clk)
        #1;
		  
        $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);
        @(posedge clk)
        #1;
		  
        $display("x_coord_adj=%d, y_coord_adj=%d", x_coord_adj, y_coord_adj);        
		  $finish;
    end

    perspective_transform dut(
        .clk(clk),
        .rst_n(rst_n),
	     .x_coord(x_coord),
	     .y_coord(y_coord),
	     .x_coord_adj(x_coord_adj),
	     .y_coord_adj(y_coord_adj)
    );

endmodule