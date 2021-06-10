`timescale 1 ns/10 ps  // time-unit = 1 ns, precision = 10 ps

module rgb_to_hsv_tb(
);
    reg clk;
    reg rst_n;
    reg[7:0] R, G, B;
    wire [8:0] hue;
    wire [7:0] sat, value;
    reg i_valid, o_valid;
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
        i_valid = 1'b0;

        @(posedge clk)
        #1;
        rst_n=1'b1;
        R = 8'd98;
        G = 8'd135;
        B = 8'd124;
//        assert(hue == 0) else $fatal(1, "hue is wrong.");
//        assert(sat == 0) else $fatal(1, "sat is wrong.");
//        assert(value == 0) else $fatal(1, "val is wrong.");
        i_valid = 1'b1;

        @(posedge clk)
        #1;
        R = 8'd78;
        G = 8'd85;
        B = 8'd255;
//        assert(hue == 0) else $fatal(1, "hue is wrong.");
//        assert(sat == 0) else $fatal(1, "sat is wrong.");
//        assert(value == 0) else $fatal(1, "value is wrong.");
//        $display("hue=%d, sat=%d, val=%d, time=%t", hue, sat, value, $time);
        
        @(posedge clk)
        #1;
        R = 8'd251;
        G = 8'd152;
        B = 8'd50;
//        assert(hue == 0) else $fatal(1, "hue is wrong.");
//        assert(sat == 0) else $fatal(1, "sat is wrong.");
//        assert(value == 0) else $fatal(1, "value is wrong.");
//        $display("hue=%d, sat=%d, val=%d, time=%t", hue, sat, value, $time);

        @(posedge clk)
        #1;
//        assert(hue == 162) else $fatal(1, "hue is wrong.");
//        assert(sat == 70) else $fatal(1, "sat is wrong.");
//        assert(value == 135) else $fatal(1, "value is wrong.");
//        $display("hue=%d, sat=%d, val=%d, time=%t", hue, sat, value, $time);
        
        @(posedge clk)
        #1;
//        assert(hue == 238) else $fatal(1, "hue is wrong.");
//        assert(sat == 177) else $fatal(1, "sat is wrong.");
//        assert(value == 255) else $fatal(1, "value is wrong.");
//        $display("hue=%d, sat=%d, val=%d, time=%t", hue, sat, value, $time);
        
        @(posedge clk)
        #1;
//        assert(hue == 30) else $fatal(1, "hue is wrong.");
//        assert(sat == 205) else $fatal(1, "sat is wrong.");
//        assert(value == 251) else $fatal(1, "value is wrong.");
//        $display("hue=%d, sat=%d, val=%d, time=%t", hue, sat, value, $time);
        
//        $display("Finished. Total time = %t", $time);
//        $finish;
    end

    rgb_to_hsv dut(
        .clk(clk),
        .rst_n(rst_n),
        .rgb_r(R),
        .rgb_g(G),
        .rgb_b(B),	
        .hsv_h(hue),
        .hsv_s(sat),
        .hsv_v(value),
        .valid_in(i_valid),
        .valid_out(o_valid)
    );

endmodule