#!/usr/bin/tclsh

# Copyright 2014 Andre Pool
# Licensed under the Apache License version 2.0
# You may not use this file except in compliance with this License
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

# Script to demonstrate connecting to the simulator environment by usage of the control_application

puts "usage of existing c control application"
set capp "../socket/control_application"

exec $capp -p "connection from tcl script at [clock format [clock seconds] -format %H:%M:%S] "
puts [ exec $capp -m "pwd" ]
puts [ exec $capp -m "date" ]
puts [ exec $capp -m "examine top/d0/sqrt" ]
puts "activate reset"
exec $capp -m "force top/rst TRUE"
puts [ exec $capp -m "examine top/d0/sqrt" ]
puts "deactivate reset"
exec $capp -m "noforce top/rst"
puts [ exec $capp -m "examine top/d0/sqrt" ]

puts "all done"


