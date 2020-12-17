# Copyright 2012-2020 Andre Pool
# Licensed under the Apache License version 2.0
# You may not use this file except in compliance with this License
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

# Run make to start the simulation environment in gui mode.
# Run make simc to start the simulation environment in batch mode.
# In both modes also the server socket interface is available.
# The client should be started from the ../socket directory.

# Run make clean to remove all generated files, including c environment, and
# stop simulator environment when still running.

# vcom can not do a parallel compile in one library (work)
MAKEFLAGS = -j1

MTI_BITS=
ifeq ($(shell uname -m), x86_64)
   MTI_BITS = -64
endif

all: sim

work/%: %.vhd
	@test -d work || vlib -quiet work
	@# to suppress vcom errors, use -suppress xx, see verror -all for list
	vcom $(MTI_BITS) -2002 -O5 -pedanticerrors -quiet $<
	@touch work/$(<:.vhd=)

work/dut: work/counter work/sqrt_int work/comparator work/ram

work/top: work/testbench work/dut

work/top_opt: work/top
	@# remove +acc for optimal performance (added for schematic view)
	@vopt $(MTI_BITS) -quiet +acc top -o top_opt

sim: c_environment work/top_opt vsim.do
	@# remove -debugdb for optimal performance (added for schematic view)
	@vsim $(MTI_BITS) -quiet -debugdb -t ns top_opt -do vsim.do
	@netstat -a | grep 2222

simc: c_environment work/top_opt vsimc.do
	@vsim $(MTI_BITS) -quiet -c -t ns top_opt -do vsimc.do
	@netstat -a | grep 2222

c_environment:
	@$(MAKE) --no-print-directory -C ../src

comp: c_environment work/top_opt

clean:
	@sync
	@killall -9 vish 2>/dev/null || true
	@killall -9 vsimk 2>/dev/null || true
	@sync
	@rm -rf work
	@rm -f vsim.dbg *.wlf *.vstf transcript
	@$(MAKE) --no-print-directory MAKEFLAGS= -C ../src clean

.PHONY: all sim simc c_environment comp clean

.SECONDARY:
