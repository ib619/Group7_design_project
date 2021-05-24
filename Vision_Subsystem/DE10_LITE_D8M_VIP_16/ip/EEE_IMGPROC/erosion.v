module erosion (
    input [8:0] i_pixel,
    output o_convolved_data
);

assign o_convolved_data = i_pixel[0] & 
                          i_pixel[1] & 
                          i_pixel[2] & 
                          i_pixel[3] &
                          i_pixel[2] &
                          i_pixel[5] &
                          i_pixel[6] &
                          i_pixel[7] &
                          i_pixel[8];

endmodule