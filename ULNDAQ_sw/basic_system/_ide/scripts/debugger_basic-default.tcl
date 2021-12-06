# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: /home/dn/ULNDAQ_sw/basic_system/_ide/scripts/debugger_basic-default.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source /home/dn/ULNDAQ_sw/basic_system/_ide/scripts/debugger_basic-default.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent JTAG-HS1 210512180081" && level==0 && jtag_device_ctx=="jsn-JTAG-HS1-210512180081-13722093-0"}
fpga -file /home/dn/ULNDAQ_sw/basic/_ide/bitstream/basic_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw /home/dn/ULNDAQ_sw/basic_wrapper/export/basic_wrapper/hw/basic_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source /home/dn/ULNDAQ_sw/basic/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow /home/dn/ULNDAQ_sw/basic/Debug/basic.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con
