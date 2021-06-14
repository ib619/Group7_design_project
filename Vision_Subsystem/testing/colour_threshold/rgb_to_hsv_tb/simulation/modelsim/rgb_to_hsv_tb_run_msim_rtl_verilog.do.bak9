transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/colour_threshold/rgb_to_hsv_tb {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/colour_threshold/rgb_to_hsv_tb/divider1.v}
vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/colour_threshold/rgb_to_hsv_tb {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/colour_threshold/rgb_to_hsv_tb/divider2.v}
vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/colour_threshold/rgb_to_hsv_tb {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/colour_threshold/rgb_to_hsv_tb/rgb_to_hsv.v}

vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/colour_threshold/rgb_to_hsv_tb {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/colour_threshold/rgb_to_hsv_tb/rgb_to_hsv_tb.v}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L fiftyfivenm_ver -L rtl_work -L work -voptargs="+acc"  rgb_to_hsv_tb

add wave *
view structure
view signals
run -all
