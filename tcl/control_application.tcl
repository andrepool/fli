#!/usr/bin/tclsh

# Copyright 2014 Andre Pool
# Licensed under the Apache License version 2.0
# You may not use this file except in compliance with this License
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

# Script to demonstrate connecting to the simulator environment by usage of the control_application

set capp "../socket/control_application"

exec make -C ../socket control_application

exec $capp -p "connection from tcl script at [clock format [clock seconds] -format %H:%M:%S] "
puts [ exec $capp -m "pwd" ]
puts [ exec $capp -m "date" ]
puts [ exec $capp -m "examine top/d0/sqrt" ]
puts "reset one clock period"
exec $capp -m "force top/rst TRUE -cancel 10"
puts [ exec $capp -m "examine top/d0/sqrt" ]

puts "all done"


