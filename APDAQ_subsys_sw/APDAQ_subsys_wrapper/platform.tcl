# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/dn/APDAQ_subsys_sw/APDAQ_subsys_wrapper/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/dn/APDAQ_subsys_sw/APDAQ_subsys_wrapper/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {APDAQ_subsys_wrapper}\
-hw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}\
-fsbl-target {psu_cortexa53_0} -out {/home/dn/APDAQ_subsys_sw}

platform write
domain create -name {freertos10_xilinx_ps7_cortexa9_0} -display-name {freertos10_xilinx_ps7_cortexa9_0} -os {freertos10_xilinx} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform active {APDAQ_subsys_wrapper}
domain active {zynq_fsbl}
domain active {freertos10_xilinx_ps7_cortexa9_0}
platform generate -quick
bsp reload
bsp setlib -name xilffs -ver 4.4
bsp config stdin "ps7_coresight_comp_0"
bsp config stdout "ps7_coresight_comp_0"
bsp config total_heap_size "134217728"
bsp config minimal_stack_size "1024"
bsp config enable_exfat "true"
bsp config set_fs_rpath "1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
bsp config stdin "ps7_uart_0"
bsp config stdout "ps7_uart_0"
bsp config max_priorities "16"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
domain active {zynq_fsbl}
bsp reload
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform active {APDAQ_subsys_wrapper}
bsp reload
bsp reload
domain active {zynq_fsbl}
bsp reload
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
platform active {APDAQ_subsys_wrapper}
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp reload
platform generate -domains 
bsp reload
bsp config stdin "ps7_coresight_comp_0"
bsp config stdout "ps7_coresight_comp_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
platform active {APDAQ_subsys_wrapper}
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform active {APDAQ_subsys_wrapper}
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {APDAQ_subsys_wrapper}
bsp reload
bsp reload
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform clean
platform generate
platform active {APDAQ_subsys_wrapper}
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform active {APDAQ_subsys_wrapper}
bsp reload
bsp write
bsp config minimal_stack_size "2048"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {APDAQ_subsys_wrapper}
bsp reload
bsp config stdin "none"
bsp config stdout "none"
bsp write
bsp reload
catch {bsp regenerate}
domain active {zynq_fsbl}
bsp reload
bsp reload
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {APDAQ_subsys_wrapper}
platform generate -domains 
platform active {APDAQ_subsys_wrapper}
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp config stdin "ps7_coresight_comp_0"
bsp config stdout "ps7_coresight_comp_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform clean
platform generate
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform active {APDAQ_subsys_wrapper}
bsp reload
bsp config stdin "none"
bsp config stdout "none"
bsp reload
platform generate -domains 
platform generate
platform active {APDAQ_subsys_wrapper}
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate
platform active {APDAQ_subsys_wrapper}
platform active {APDAQ_subsys_wrapper}
platform active {APDAQ_subsys_wrapper}
platform generate
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp config stdin "ps7_uart_0"
bsp config stdout "ps7_uart_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {APDAQ_subsys_wrapper}
platform generate -domains 
bsp reload
bsp config stdin "ps7_coresight_comp_0"
bsp config stdout "ps7_coresight_comp_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/APDAQ_subsys/APDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform active {APDAQ_subsys_wrapper}
bsp reload
bsp config stdin "ps7_uart_0"
bsp config stdout "ps7_uart_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {APDAQ_subsys_wrapper}
bsp reload
bsp config use_lfn "1"
bsp write
bsp reload
catch {bsp regenerate}
platform active {APDAQ_subsys_wrapper}
platform generate
bsp reload
bsp config stdin "ps7_coresight_comp_0"
bsp config stdout "ps7_coresight_comp_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
bsp config use_chmod "true"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {APDAQ_subsys_wrapper}
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp reload
platform generate
bsp reload
bsp reload
bsp config tick_rate "1000"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {APDAQ_subsys_wrapper}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0,zynq_fsbl 
platform active {APDAQ_subsys_wrapper}
bsp reload
bsp reload
platform active {APDAQ_subsys_wrapper}
platform generate
platform active {APDAQ_subsys_wrapper}
bsp reload
bsp reload
platform generate -domains 
platform generate
platform active {APDAQ_subsys_wrapper}
domain active {zynq_fsbl}
bsp reload
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp config stdin "ps7_uart_0"
bsp config stdout "ps7_uart_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {APDAQ_subsys_wrapper}
