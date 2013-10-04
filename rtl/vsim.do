# Copyright 2012, 2013 Andre Pool
# Licensed under the Apache License version 2.0
# You may not use this file except in compliance with this License
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

# Simulation startup script for gui mode.
onerror {resume}
alias q "quit -f"

force -freeze sim:/top/d0/m0/ram_s(30) 32'h00000030 0
force -freeze sim:/top/d0/m0/ram_s(31) 32'h00000031 0
force -freeze sim:/top/d0/m0/ram_s(32) 32'h00000032 0
force -freeze sim:/top/d0/m0/ram_s(33) 32'h00000033 0
force -freeze sim:/top/d0/m0/ram_s(34) 32'h00000034 0
force -freeze sim:/top/d0/m0/ram_s(35) 32'h00000035 0
force -freeze sim:/top/d0/m0/ram_s(36) 32'h00000036 0
force -freeze sim:/top/d0/m0/ram_s(37) 32'h00000037 0
force -freeze sim:/top/d0/m0/ram_s(38) 32'h00000038 0
force -freeze sim:/top/d0/m0/ram_s(39) 32'h00000039 0

add wave top/clk
add wave -color yellow top/rst
add wave -color purple top/set
add wave top/inc
add wave top/dec
add wave -radix unsigned top/load
add wave -radix unsigned -color gold -format analog-step -min 0 -max 400 -height 100 top/d0/cnt
add wave -radix unsigned -color gold -format analog-step -min 0 -max 20 -height 100 top/d0/sqrt
add wave top/d0/cmp
add wave -radix unsigned -color pink top/d0/read_address
add wave -radix hex -color blue top/d0/read_data
run -all
wave zoomfull
