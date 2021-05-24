module line_buffer(
    input clk,
    input rst_n,
    input i_data,
    input i_data_valid,
    input i_rd_data,
    output [2:0] o_data
);

localparam width = 640;

reg line [width + 1:0];
reg [9:0] wrPtr;
reg [9:0] rdPtr;
reg i_valid_delayed;

integer i;
initial
    begin
        for(i=0;i<width + 2;i=i+1)
        begin
            line[i] <= 1'b1;
        end
        wrPtr <= 10'd1;
        rdPtr <= 10'd1;
    end

always @(posedge clk)
begin
    if(i_data_valid)
        line[wrPtr] <= i_data;
end


always @(posedge clk)
begin  
    begin
    if (~rst_n | (wrPtr == width & i_data_valid))
        wrPtr <= 10'd1;
    else if (wrPtr < width &  i_data_valid)
        wrPtr <= wrPtr + 10'd1;
    end
end

always @(posedge clk)
begin 
    begin
    if (~rst_n | (rdPtr == width & i_rd_data ))
        rdPtr <= 10'd1;
    else if (rdPtr < width & i_rd_data )
        rdPtr <= rdPtr + 10'd1;
    end
end

assign o_data = {line[rdPtr-1], line[rdPtr ], line[rdPtr+1]};

endmodule