transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/morph_filter/erosion {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/morph_filter/erosion/convolution3x3_erosion.v}
vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/morph_filter/erosion {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/morph_filter/erosion/line_buffer3x3_1bit.v}

vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/morph_filter/erosion {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/morph_filter/erosion/erosion_filter_tb.v}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L fiftyfivenm_ver -L rtl_work -L work -voptargs="+acc"  erosion_filter_tb

add wave *
view structure
view signals
run -all
