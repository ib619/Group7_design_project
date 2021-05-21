module i2c_adapter(
    inout tri1 sda,
    inout tri1 scl,

    output wire mg_sda_in,
    output wire mg_scl_in,

    input wire mg_sda_oe,
    input wire mg_scl_oe
);
    assign mg_sda_in = sda;
    assign mg_scl_in = scl;
    assign sda = mg_sda_oe ? 1'b0 : 1'bz;
    assign scl = mg_scl_oe ? 1'b0 : 1'bz;

endmodule