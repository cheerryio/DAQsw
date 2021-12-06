# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/dn/LPDAQ_subsys_sw/LPDAQ_subsys3_wrapper/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/dn/LPDAQ_subsys_sw/LPDAQ_subsys3_wrapper/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {LPDAQ_subsys3_wrapper}\
-hw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}\
-fsbl-target {psu_cortexa53_0} -out {/home/dn/LPDAQ_subsys_sw}

platform write
domain create -name {freertos10_xilinx_ps7_cortexa9_0} -display-name {freertos10_xilinx_ps7_cortexa9_0} -os {freertos10_xilinx} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform active {LPDAQ_subsys3_wrapper}
domain active {zynq_fsbl}
domain active {freertos10_xilinx_ps7_cortexa9_0}
platform generate -quick
bsp reload
bsp setlib -name xilffs -ver 4.4
bsp config enable_exfat "true"
bsp config set_fs_rpath "1"
bsp config stdin "ps7_coresight_comp_0"
bsp config tick_rate "1000"
bsp config total_heap_size "134217728"
bsp config minimal_stack_size "1024"
bsp config max_priorities "16"
bsp write
bsp reload
catch {bsp regenerate}
domain active {zynq_fsbl}
bsp reload
bsp reload
platform generate
platform active {LPDAQ_subsys3_wrapper}
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {LPDAQ_subsys3_wrapper}
platform generate -domains 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {LPDAQ_subsys3_wrapper}
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform active {LPDAQ_subsys3_wrapper}
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform generate
platform active {LPDAQ_subsys3_wrapper}
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
platform generate -domains 
platform active {LPDAQ_subsys3_wrapper}
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform clean
platform active {LPDAQ_subsys3_wrapper}
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate
bsp reload
bsp config stdin "ps7_uart_0"
bsp config stdout "ps7_uart_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {LPDAQ_subsys3_wrapper}
bsp reload
bsp config stdin "ps7_coresight_comp_0"
bsp config stdout "ps7_coresight_comp_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
platform active {LPDAQ_subsys3_wrapper}
bsp reload
bsp reload
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform active {LPDAQ_subsys3_wrapper}
bsp reload
platform generate -domains 
platform active {LPDAQ_subsys3_wrapper}
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform generate -domains 
bsp reload
bsp reload
platform generate -domains 
platform clean
platform generate
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform active {LPDAQ_subsys3_wrapper}
bsp reload
bsp reload
platform generate -domains 
platform active {LPDAQ_subsys3_wrapper}
bsp reload
bsp reload
platform generate
platform active {LPDAQ_subsys3_wrapper}
platform config -updatehw {/home/dn/LPDAQ_subsys_hw/LPDAQ_subsys3_wrapper.xsa}
platform generate -domains 
platform clean
platform generate
bsp reload
bsp config total_heap_size "6553600"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
