transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/perspective_transform/perspective_transform_tb {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/perspective_transform/perspective_transform_tb/signed_divide_delay.v}
vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/perspective_transform/perspective_transform_tb {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/perspective_transform/perspective_transform_tb/perspective_transform.v}

vlog -vlog01compat -work work +incdir+C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/perspective_transform/perspective_transform_tb {C:/Users/tansi/Documents/Imperial_College_London/Year2_Project/Group7_design_project/Vision_Subsystem/testing/perspective_transform/perspective_transform_tb/perspective_transform_tb.v}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L fiftyfivenm_ver -L rtl_work -L work -voptargs="+acc"  perspective_transform_tb

add wave *
view structure
view signals
run -all
