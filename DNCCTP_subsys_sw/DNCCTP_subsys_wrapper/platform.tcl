# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/dn/DNCCTP_subsys_sw/DNCCTP_subsys_wrapper/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/dn/DNCCTP_subsys_sw/DNCCTP_subsys_wrapper/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {DNCCTP_subsys_wrapper}\
-hw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}\
-fsbl-target {psu_cortexa53_0} -out {/home/dn/DNCCTP_subsys_sw}

platform write
domain create -name {freertos10_xilinx_ps7_cortexa9_0} -display-name {freertos10_xilinx_ps7_cortexa9_0} -os {freertos10_xilinx} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform active {DNCCTP_subsys_wrapper}
domain active {zynq_fsbl}
domain active {freertos10_xilinx_ps7_cortexa9_0}
platform generate -quick
bsp reload
bsp setlib -name lwip211 -ver 1.3
bsp config api_mode "SOCKET_API"
bsp config stdin "ps7_coresight_comp_0"
bsp config stdout "ps7_coresight_comp_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
platform active {DNCCTP_subsys_wrapper}
bsp reload
bsp config total_heap_size "134217728"
bsp config tick_rate "100"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
platform clean
platform generate
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform clean
platform generate
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform active {DNCCTP_subsys_wrapper}
bsp reload
bsp reload
bsp reload
platform generate -domains 
platform active {DNCCTP_subsys_wrapper}
bsp reload
bsp reload
platform active {DNCCTP_subsys_wrapper}
platform generate -domains 
platform active {DNCCTP_subsys_wrapper}
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform generate
platform active {DNCCTP_subsys_wrapper}
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform active {DNCCTP_subsys_wrapper}
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform generate -domains 
platform generate
platform active {DNCCTP_subsys_wrapper}
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform generate
platform generate
platform generate
platform active {DNCCTP_subsys_wrapper}
bsp reload
bsp config stdin "ps7_uart_0"
bsp config stdout "ps7_uart_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {DNCCTP_subsys_wrapper}
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
bsp reload
bsp reload
platform generate -domains 
platform active {DNCCTP_subsys_wrapper}
platform config -updatehw {/home/dn/DNCCTP_subsys_hw/DNCCTP_subsys_wrapper.xsa}
platform generate -domains 
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp config stdin "ps7_uart_0"
bsp reload
platform generate
