// (C) 2001-2016 Intel Corporation. All rights reserved.
// Your use of Intel Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Intel Program License Subscription 
// Agreement, Intel MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Intel and sold by 
// Intel or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


module Qsys_alt_vip_cl_2dfir_0_alg_core

   (  input    wire                                av_st_cmd_valid,
      input    wire                                av_st_cmd_startofpacket,
      input    wire                                av_st_cmd_endofpacket,
      input    wire  [64 - 1 : 0]     av_st_cmd_data,
      output   wire                                av_st_cmd_ready,
      
      input    wire                                av_st_din_valid,
      input    wire                                av_st_din_startofpacket,
      input    wire                                av_st_din_endofpacket,
      input    wire  [152 - 1 : 0]     av_st_din_data,
      output   wire                                av_st_din_ready,
      
      output   wire                                av_st_dout_valid,
      output   wire                                av_st_dout_startofpacket,
      output   wire                                av_st_dout_endofpacket,
      output   wire  [56 - 1 : 0]    av_st_dout_data,
      input    wire                                av_st_dout_ready,
      
      
      input    wire   clock,
      input    wire   reset
   );
   
   localparam  integer  FIXED_COEFFS  [0 : 80] =  '{120,  480,  840,  1920,  3121,  4921,  0,  0,  0,
                                                    0,  0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0, 0};

   wire                                av_st_coeff_valid;
   wire                                av_st_coeff_startofpacket;
   wire                                av_st_coeff_endofpacket;
   wire  [1 - 1 : 0]   av_st_coeff_data;
   wire                                av_st_coeff_ready;

   alt_vip_fir_alg_core # (
      .NUMBER_OF_COLOR_PLANES          (3),
      .COLOR_PLANES_ARE_IN_PARALLEL    (1),
      .BITS_PER_SYMBOL_IN              (8),
      .IS_422                          (0),
      .BITS_PER_SYMBOL_OUT             (8),
      .PIXELS_IN_PARALLEL              (1),
      .MOVE_BINARY_POINT_RIGHT         (0),
      .ROUNDING_METHOD                 ("ROUND_HALF_UP"),
      .EDGE_ADAPTIVE_SHARPEN           (0),
      .DO_MIRRORING                    (1),
      .ENABLE_WIDE_BLUR_SHARPEN        (1),
      .RUNTIME_CONTROL                 (0),
      .UPPER_BLUR_LIM                  (15),
      .LOWER_BLUR_LIM                  (0),
      .H_TAPS                          (5),
      .V_TAPS                          (5),
      .COEFF_SIGNED                    (0),
      .COEFF_INTEGER_BITS              (1),
      .COEFF_FRACTION_BITS             (15),
      .V_SYMMETRIC                     (1),
      .H_SYMMETRIC                     (1),
      .DIAG_SYMMETRIC                  (1),
      .SRC_WIDTH                       (8),
      .DST_WIDTH                       (8),
      .CONTEXT_WIDTH                   (8),
      .TASK_WIDTH                      (8),
      .SOURCE_ID                       (0),
      .PIPELINE_READY                  (0),
      .PRE_ALIGNED_SOP                 (1),
      .CYCLONE_STYLE                   (0),
      .V_SERIES_STYLE                  (0),
      .FIXED_COEFFS                    (FIXED_COEFFS)
   ) fir_core_inst (
      .clock                           (clock),
      .reset                           (reset),
      .av_st_cmd_ready                 (av_st_cmd_ready),
      .av_st_cmd_valid                 (av_st_cmd_valid),
      .av_st_cmd_startofpacket         (av_st_cmd_startofpacket),
      .av_st_cmd_endofpacket           (av_st_cmd_endofpacket),
      .av_st_cmd_data                  (av_st_cmd_data),
      .av_st_din_ready                 (av_st_din_ready),
      .av_st_din_valid                 (av_st_din_valid),
      .av_st_din_startofpacket         (av_st_din_startofpacket),
      .av_st_din_endofpacket           (av_st_din_endofpacket),
      .av_st_din_data                  (av_st_din_data),
      .av_st_dout_ready                (av_st_dout_ready),
      .av_st_dout_valid                (av_st_dout_valid),
      .av_st_dout_startofpacket        (av_st_dout_startofpacket),
      .av_st_dout_endofpacket          (av_st_dout_endofpacket),
      .av_st_dout_data                 (av_st_dout_data),
      .av_st_coeff_ready               (av_st_coeff_ready),
      .av_st_coeff_valid               (av_st_coeff_valid),
      .av_st_coeff_startofpacket       (av_st_coeff_startofpacket),
      .av_st_coeff_endofpacket         (av_st_coeff_endofpacket),
      .av_st_coeff_data                (av_st_coeff_data)
   );

endmodule

