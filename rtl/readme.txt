# Copyright 2012, 2013 Andre Pool
# Licensed under the Apache License version 2.0
# you may not use this file except in compliance with this License
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#### compile and run output log of server ####

fli/rtl$ make simc
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../socket -I$MTI_HOME/include -o testbench.o -c testbench.c
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../socket -I$MTI_HOME/include -o counter.o -c counter.c
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../socket -I$MTI_HOME/include -o sqrt_int.o -c sqrt_int.c
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../socket -I$MTI_HOME/include -o global_var.o -c global_var.c
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../socket -I$MTI_HOME/include -o housekeeping.o -c housekeeping.c
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../socket -I$MTI_HOME/include -o server.o -c server.c
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../socket -I$MTI_HOME/include -o sock_functions.o -c ../socket/sock_functions.c
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../socket -I$MTI_HOME/include -o server_functions.o -c ../socket/server_functions.c
gcc -shared -Wl,-Bsymbolic -Wl,-export-dynamic -std=c99 -m32 -o c_environment.so testbench.o counter.o sqrt_int.o global_var.o housekeeping.o server.o sock_functions.o server_functions.o -lm -lpthread
vcom  -2002 -O5 -pedanticerrors -quiet testbench.vhd
vcom  -2002 -O5 -pedanticerrors -quiet counter.vhd
vcom  -2002 -O5 -pedanticerrors -quiet sqrt_int.vhd
vcom  -2002 -O5 -pedanticerrors -quiet comparator.vhd
vcom  -2002 -O5 -pedanticerrors -quiet dut.vhd
vcom  -2002 -O5 -pedanticerrors -quiet top.vhd
Reading /mtitcl/vsim/pref.tcl 

# 10.2

# vsim -do vsimc.do -c -quiet -t ns top_opt 
# //  Questa Sim
# //  Version 10.2 linux Feb  2 2013
# //
# //  Copyright 1991-2013 Mentor Graphics Corporation
# //  All Rights Reserved.
# //
# //  THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION
# //  WHICH IS THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS
# //  LICENSORS AND IS SUBJECT TO LICENSE TERMS.
# //
# INFO    server with API version 10
# INFO    wait for client connection
# do vsimc.do 
# INFO    client connected to server for the 0x00000000 time
####    first message from control_application ####
# wave zoomrange not supported in batch mode
# wave zoomrange not supported in batch mode
# wave zoomrange not supported in batch mode
# wave zoomrange not supported in batch mode
# wave zoomrange not supported in batch mode
# wave zoomrange not supported in batch mode
# wave zoomrange not supported in batch mode
# wave zoomrange not supported in batch mode
# wave zoomrange not supported in batch mode
# wave zoomrange not supported in batch mode
# INFO    set sum from 12 to 13
# INFO    incr sum from 13 to 14
####    last  message from control_application ####
####    now shutdown ####
# WARNING simulator quit requested by configuration application


#### compile and run output log of client ####
fli/socket$ make
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../src -I$MTI_HOME/include -o control_application.o -c control_application.c
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -I. -I../src -I$MTI_HOME/include -o sock_functions.o -c sock_functions.c
gcc -g -O2 -Wall -ansi -fms-extensions -std=c99 -pedantic -m32 -freg-struct-return -o control_application control_application.o sock_functions.o -lm
INFO    server API version 10
INFO    buffer size 0.524 Mbytes
INFO    mti version: 'ModelSim for Questa  Version 10.2 2013.02'
INFO    current count value   56 current sqrt value  11
INFO    get sum 12 from server, after sum + 1 and inc, sum gets 14
INFO    mti cmd 'date' returns 'xxx'
INFO    iteration            0, current time            9,724,445 ns
INFO    mti cmd 'pwd' returns 'xxx/fli/rtl'
INFO    iteration            1, current time            9,862,495 ns
INFO    iteration            2, current time            9,975,100 ns
INFO    iteration            3, current time           10,083,935 ns
INFO    iteration            4, current time           10,191,190 ns
INFO    iteration            5, current time           10,293,040 ns
INFO    iteration            6, current time           10,371,970 ns
INFO    iteration            7, current time           10,437,060 ns
INFO    iteration            8, current time           10,508,575 ns
INFO    iteration            9, current time           10,579,800 ns
INFO    client all done connecting 10 times to the server
INFO    done with testing, send one last message to server
fli/socket$ make quit
INFO    control application request to quit simulator
INFO    response 'simulator has stopped'
