# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/dn/SDPDAQ_subsys_sw/bd_wrapper/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/dn/SDPDAQ_subsys_sw/bd_wrapper/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {bd_wrapper}\
-hw {/home/dn/SDPDAQ_subsys_hw/bd_wrapper.xsa}\
-fsbl-target {psu_cortexa53_0} -out {/home/dn/SDPDAQ_subsys_sw}

platform write
domain create -name {freertos10_xilinx_ps7_cortexa9_0} -display-name {freertos10_xilinx_ps7_cortexa9_0} -os {freertos10_xilinx} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform active {bd_wrapper}
domain active {zynq_fsbl}
domain active {freertos10_xilinx_ps7_cortexa9_0}
platform generate -quick
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp config stdin "ps7_uart_1"
bsp config stdout "ps7_uart_1"
bsp config total_heap_size "16777216"
bsp config tick_rate "1000"
bsp config minimal_stack_size "1024"
bsp config max_priorities "16"
bsp setlib -name xilffs -ver 4.4
bsp config enable_exfat "true"
bsp config use_chmod "true"
bsp config set_fs_rpath "1"
bsp config use_lfn "1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
platform config -updatehw {/home/dn/SDPDAQ_subsys_hw/bd_wrapper.xsa}
platform generate -domains 
platform active {bd_wrapper}
platform config -updatehw {/home/dn/SDPDAQ_subsys_hw/bd_wrapper.xsa}
platform generate -domains 
bsp reload
bsp reload
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {bd_wrapper}
bsp reload
bsp reload
platform active {bd_wrapper}
platform generate
platform clean
platform generate
platform active {bd_wrapper}
bsp reload
bsp config total_heap_size "1677721600"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp config total_heap_size "16777216"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {bd_wrapper}
domain create -name {standalone_ps7_cortexa9_0} -display-name {standalone_ps7_cortexa9_0} -os {standalone} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {memory_tests}
platform generate -domains 
platform write
domain active {zynq_fsbl}
domain active {freertos10_xilinx_ps7_cortexa9_0}
domain active {standalone_ps7_cortexa9_0}
platform generate -quick
platform generate -domains standalone_ps7_cortexa9_0 
domain active {standalone_ps7_cortexa9_0}
bsp reload
bsp config stdin "ps7_uart_1"
bsp config stdout "ps7_uart_1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_ps7_cortexa9_0 
