#!/usr/bin/tclsh

# Copyright 2014 Andre Pool
# Licensed under the Apache License version 2.0
# You may not use this file except in compliance with this License
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

exec make -C ../socket control_application

puts "send break signal to simulator"
set capp "../socket/control_application"

exec $capp -p "connection from tcl script at [clock format [clock seconds] -format %H:%M:%S] "
exec $capp -b

puts "all done"


