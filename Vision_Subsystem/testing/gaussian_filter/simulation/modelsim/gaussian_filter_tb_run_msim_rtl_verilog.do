transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/gaussian_filter {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/gaussian_filter/separableconvolution5x5_gaussian.v}
vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/gaussian_filter {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/gaussian_filter/line_buffer5x5_8bit.v}

vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/gaussian_filter {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/test/Group7_design_project/Vision_Subsystem/testing/gaussian_filter/gaussian_filter_tb.v}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L fiftyfivenm_ver -L rtl_work -L work -voptargs="+acc"  gaussian_filter_tb

add wave *
view structure
view signals
run -all
