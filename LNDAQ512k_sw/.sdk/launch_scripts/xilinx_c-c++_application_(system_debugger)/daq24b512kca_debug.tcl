connect -url tcp:127.0.0.1:3121
targets -set -filter {jtag_cable_name =~ "Digilent JTAG-HS1 210205741823A" && level==0} -index 0
fpga -file /home/ubt/xilinx/duna_daq1245123_rev1_1_1803/duna_daq1245123_rev1_1_1803.sdk/ll7a35ek_mbsys_wrapper_hw_platform_0/ll7a35ek_mbsys_wrapper.bit
configparams mdm-detect-bscan-mask 2
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-HS1 210205741823A"} -index 0
rst -system
after 3000
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-HS1 210205741823A"} -index 0
dow /home/ubt/xilinx/duna_daq1245123_rev1_1_1803/duna_daq1245123_rev1_1_1803.sdk/daq24b512kca/Debug/daq24b512kca.elf
bpadd -addr &main
