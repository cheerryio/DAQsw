# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/dn/ULNDAQ_sw/basic_wrapper/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/dn/ULNDAQ_sw/basic_wrapper/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {basic_wrapper}\
-hw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}\
-fsbl-target {psu_cortexa53_0} -out {/home/dn/ULNDAQ_sw}

platform write
domain create -name {freertos10_xilinx_ps7_cortexa9_0} -display-name {freertos10_xilinx_ps7_cortexa9_0} -os {freertos10_xilinx} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform active {basic_wrapper}
domain active {zynq_fsbl}
domain active {freertos10_xilinx_ps7_cortexa9_0}
platform generate -quick
platform generate
bsp reload
bsp config stdin "ps7_coresight_comp_0"
bsp config stdout "ps7_coresight_comp_0"
bsp config total_heap_size "6553600"
bsp config tick_rate "1000"
bsp config minimal_stack_size "1024"
bsp config max_task_name_len "10"
bsp config max_priorities "16"
bsp config software_timers "true"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp setlib -name xilffs -ver 4.4
bsp config enable_exfat "true"
bsp config use_chmod "true"
bsp config set_fs_rpath "1"
bsp config use_lfn "1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform active {basic_wrapper}
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
bsp reload
bsp config total_heap_size "655360"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
platform clean
platform generate
bsp config total_heap_size "6553600"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0,zynq_fsbl 
platform active {basic_wrapper}
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
bsp reload
bsp config total_heap_size "655360"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform generate
platform generate
platform active {basic_wrapper}
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp config total_heap_size "655361"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {basic_wrapper}
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp reload
platform generate -domains 
platform active {basic_wrapper}
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
bsp reload
bsp config total_heap_size "655360"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
bsp reload
bsp reload
bsp config tick_rate "100"
bsp write
bsp reload
catch {bsp regenerate}
bsp config tick_rate "1000"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {basic_wrapper}
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
bsp reload
bsp config total_heap_size "655361"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform active {basic_wrapper}
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
bsp reload
bsp config total_heap_size "655360"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
bsp reload
bsp config total_heap_size "655361"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate -domains 
platform generate
platform active {basic_wrapper}
bsp reload
bsp config stdin "ps7_uart_0"
bsp config stdout "ps7_uart_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {basic_wrapper}
platform active {basic_wrapper}
platform active {basic_wrapper}
platform config -updatehw {/home/dn/ULNDAQ_hw/basic_wrapper.xsa}
platform generate
