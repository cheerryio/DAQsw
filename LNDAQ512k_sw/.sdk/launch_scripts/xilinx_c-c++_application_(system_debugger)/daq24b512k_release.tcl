connect -url tcp:127.0.0.1:3121
targets -set -filter {jtag_cable_name =~ "Digilent JTAG-HS1 210205741823A" && level==0} -index 0
fpga -file /home/loywong/projects/xilinx/duna_daq512k/duna_daq512k.sdk/ll7a35ek_mbsys_wrapper_hw_platform_0/ll7a35ek_mbsys_wrapper.bit
configparams mdm-detect-bscan-mask 2
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-HS1 210205741823A"} -index 0
rst -system
after 3000
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-HS1 210205741823A"} -index 0
dow /home/loywong/projects/xilinx/duna_daq512k/duna_daq512k.sdk/daq24b512k/Release/daq24b512k.elf
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-HS1 210205741823A"} -index 0
con
