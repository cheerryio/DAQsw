# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/dn/DNCDAQ_subsys_sw/DNCDAQ_subsys_wrapper/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/dn/DNCDAQ_subsys_sw/DNCDAQ_subsys_wrapper/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {DNCDAQ_subsys_wrapper}\
-hw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}\
-fsbl-target {psu_cortexa53_0} -out {/home/dn/DNCDAQ_subsys_sw}

platform write
domain create -name {freertos10_xilinx_ps7_cortexa9_0} -display-name {freertos10_xilinx_ps7_cortexa9_0} -os {freertos10_xilinx} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform active {DNCDAQ_subsys_wrapper}
domain active {zynq_fsbl}
domain active {freertos10_xilinx_ps7_cortexa9_0}
platform generate -quick
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp setlib -name lwip211 -ver 1.3
bsp config api_mode "SOCKET_API"
bsp config stdin "ps7_coresight_comp_0"
bsp config stdout "ps7_coresight_comp_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp config lwip_tcp_keepalive "true"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp config api_mode "SOCKET_API"
bsp reload
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform generate
platform clean
platform active {DNCDAQ_subsys_wrapper}
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform active {DNCDAQ_subsys_wrapper}
bsp reload
bsp config tick_rate "1000"
bsp write
bsp reload
catch {bsp regenerate}
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
platform active {DNCDAQ_subsys_wrapper}
bsp reload
bsp config minimal_stack_size "1024"
bsp config max_priorities "16"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp config socket_mode_thread_prio "4"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp reload
bsp reload
bsp reload
platform active {DNCDAQ_subsys_wrapper}
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform active {DNCDAQ_subsys_wrapper}
bsp reload
bsp reload
platform generate -domains 
bsp config total_heap_size "65536"
bsp config total_heap_size "9130660"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp config tcp_wnd "65535"
bsp config tcp_snd_buf "65535"
bsp write
bsp reload
catch {bsp regenerate}
bsp config tcp_snd_buf "16384"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
platform active {DNCDAQ_subsys_wrapper}
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
bsp reload
bsp reload
platform generate -domains 
platform active {DNCDAQ_subsys_wrapper}
bsp reload
bsp config socket_mode_thread_prio "2"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform active {DNCDAQ_subsys_wrapper}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
domain active {zynq_fsbl}
bsp reload
bsp reload
bsp reload
domain active {freertos10_xilinx_ps7_cortexa9_0}
bsp reload
bsp config socket_mode_thread_prio "4"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {DNCDAQ_subsys_wrapper}
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
bsp reload
bsp reload
platform generate -domains 
platform config -updatehw {/home/dn/DNCDAQ_subsys_hw/DNCDAQ_subsys_wrapper.xsa}
platform generate -domains 
platform generate
platform active {DNCDAQ_subsys_wrapper}
bsp reload
bsp config socket_mode_thread_prio "2"
bsp config socket_mode_thread_prio "4"
bsp reload
bsp config socket_mode_thread_prio "2"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
bsp config stdin "ps7_uart_0"
bsp config stdout "ps7_uart_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_ps7_cortexa9_0 
platform active {DNCDAQ_subsys_wrapper}
platform generate -domains 
platform generate
platform active {DNCDAQ_subsys_wrapper}
bsp reload
bsp reload
platform active {DNCDAQ_subsys_wrapper}
platform active {DNCDAQ_subsys_wrapper}
platform generate
