#!/usr/bin/tclsh

proc str2bin32 { string } {
   # convert string to hexadecimal value of 8 characters
#   set hex [ format %08x $string ]

   # convert hex string to binary format
#   set bin [ binary format H8 $hex ]

   set bin [ binary format i $string ]

   return $bin
}


set ch [open "|netcat -U sock_file_handle" r+]

# read data from file
# set fp [open "|netcat -U sock_file_handle" r+]
# "|nc -U ..." r+]

fconfigure $ch -translation binary
  fconfigure $ch -buffering line
  

set command 3
# {1 0}

set outBinData [binary format s2Sa6B8 {100 -2} 100 foobar 01000001]

puts "Format done: $outBinData"

# s2 = string array 2
# S = string
# a6 = ascii 6
# B8 = binairy 8
set blaat [binary format s2Sa6B8 {100 -2} 100 foobar 01000001]

puts "blaat: $blaat"

set blaat2 [binary format S2 { 43 53 } ]

puts "blaat2: $blaat2"

set blaat3 [binary format H8 abcd0123]
puts "blaat3: $blaat3 xx"



# eerste string, moet 8 bytes worden


set first  1094861636
# 555819297
# 3405700781 
# 448428271720
#  1751672936
#  6842472 

set first_hex [ format %08x $first ]

set first_bin [ binary format h8 $first_hex ]

puts "first $first $first_hex $first_bin xx"

puts [ str2bin32 $first ]

# tweede string moet 8 bytes worden
# set second [binary format S


# puts -nonewline $command


# puts $ch $command

# set data [read $ch 4]

# set data [read $fp]

# puts -nonewline $data
# set buf_size [ 5 ]

set struct { 1 20 9 0 131072 }
# set cmd_puts { [ str2bin32 $cmd[0] ]  [ str2bin32 $cmd[0] ] }

set newlist ""
foreach field $struct {

    set new_field [ str2bin32 $field ]
    puts $new_field
    append newlist $new_field
    
}
puts "xx $newlist xx"
set readable [ binary format a20 $newlist ]
puts "readable xx $readable xx"

puts -nonewline $ch $newlist

puts "read back data"
# gets $ch line
# puts <-$line

puts "done"

close $ch
