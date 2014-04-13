#!/usr/bin/env python2

# Copyright 2014 Andre Pool
# Licensed under the Apache License version 2.0
# you may not use this file except in compliance with this License
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

# Andre WARNING: the payload has not yet been implemented, this limits the functionality to
# commands that only use the header (e.g. API check)

import socket
import sys
import struct
import time

API_VERSION = 12
SOCK_FILE_HANDLE = "../socket/sock_file_handle"

# Command list and response list.
# This list is used by client to send commands to the server, and by server to 
# send a response back to client.
# The list is subject to change, however the position of the first two elements may
# not be changed to guarantee that the correct API version can be resolved.
# When the list is changed, the API_VERSION number should be increased.

# command_response_list = [
# Andre fixme: find a way to do this dynamic in python with some kind of enum list
# if needed  the list can be generated from the sock_config.h file
AA_DO_NOT_USE_ZERO = 0
API_GET = 1 # keep at 1
COUNTER_GET = 2
DISCONNECT = 3 # sending disconnect is same as transmitting 0 bytes from the client to server
ERROR = 4
ERROR_MESSAGE_SIZE = 5
MTI_BREAK = 6
MTI_CMD = 7 # result not transcribed in transcript window but returned to control application
MTI_COMMAND = 8 # result in vsim transcript window
MTI_FIRST_LOWER_REGION = 9
MTI_GET_LIBRARY_NAME = 10
MTI_GET_PRIMARY_NAME = 11 # entity, package or module name
MTI_GET_REGION_NAME = 12 # instantiation
MTI_GET_REGION_FULL_NAME = 13 # hierarchical name of instantiation
MTI_GET_REGION_SOURCE_NAME = 14 # source file which contains instantiation
MTI_GET_SECONDARY_NAME = 15 # architecture name
MTI_GET_TOP_REGION = 16
MTI_NEXT_REGION = 17
MTI_PROC_TEST1 = 18
MTI_QUIT = 19
MTI_VERSION_GET = 20
OKAY = 21
PAYLOAD_READ = 22
PAYLOAD_WRITE = 23
SEND_EMPTY_MESSAGE = 24 # sending 0 bytes, same server behavior as disconnect, only for testing
SQRT_INT_GET = 25
SUM_INC = 26
SUM_GET = 27
SUM_SET = 28
TIME_GET_NOW = 29
TIME_GET_RES = 30
TRANSCRIPT_PRINT = 31
ZZ_DO_NOT_USE_LAST = 32

# The simulator can only be stopped when the command and the magic are correct.
MTI_QUIT_MAGIC = 0xabcd0011

# A sock_buf_t struct is used to assemble a message to and from the server.
# The struct message exists of a header and payload.

# For x86-64 optimal usage the payload should start at a 64 bit boundery

# The header exists of four 32 bit words:
# - command, which also can represent a response
# - size in bytes of the message (header + payload)
# - addr used when a specific (start) address is required probably used together with
#   the read and write command, for write the amount of bytes = size - header, for
#   read the amount of bytes is requested by the first payload word u32[0]
# - reserved field (payload start at 64 bit boundary)

SOCK_BUF_PAYLOAD_SIZE = 2048

# Andre fixme: for the time being, use class, however no idea how to add the payload to this closs
# Andre discuss: the c struct was actuall for c optimization, to prevent any data copy for the
# send and receive functions, in python everthing needs to be casted anyway, so this
# optimization does not make sense, choose anything that matches best with python
# and is compatabile with the server
# Andre disuss: maybe also use classes in python to make things more simple
# The header should be 64 bit alligned to be compatible with efficient x86-64 c code on server side
class sock_buf_class:
  command = 0
  size = 0 # message size in bytes
  addr = 0
  reserved = 0
  pl = ""
  pl_u32_0 = ""
  pl_u32_1 = ""

SOCK_BUF_HEADER_SIZE = 4*4

def sock_transmit_data( sock_file_desc, buf ) :
   # print "INFO    tr cd 0x%08x, sz 0x%08x, ad 0x%08x" % \
   #   (buf.command, buf.size, buf.addr ), "pl ", buf.pl
   # Andre fixme: find a way to also pack the payload, or at least do something with it
   tx_data = struct.pack('IIII', buf.command, buf.size,  buf.addr, 0)
   if buf.size == SOCK_BUF_HEADER_SIZE :
      buf.pl = ""
   
   tx_data += buf.pl
   # Andre: check if flags are needed for the sendall function, e.g. MSG_NOSIGNAL
   sock_file_desc.sendall( tx_data )

def sock_receive_data( sock_file_desc ) :
   # Andre check flags recv
   rx_data = sock_file_desc.recv( SOCK_BUF_HEADER_SIZE + SOCK_BUF_PAYLOAD_SIZE )
   buf = sock_buf_class() # create buffer of correct type
   (buf.command,) = struct.unpack_from('<I', rx_data)
   (buf.size,) = struct.unpack_from('<I', rx_data, 4)
   (buf.addr,) = struct.unpack_from('<I', rx_data, 8)
   buf.pl = rx_data[SOCK_BUF_HEADER_SIZE:]
   # for convenience decode already the first and second u32 words if they are available in the payload
   if len(buf.pl) == 4 :
      (buf.pl_u32_0,) = struct.unpack_from('<I', rx_data, SOCK_BUF_HEADER_SIZE)  
   if len(buf.pl) == 8 :
      (buf.pl_u32_0,buf.pl_u32_1,) = struct.unpack_from('<II', rx_data, SOCK_BUF_HEADER_SIZE)

   # print "INFO    rx cmd 0x%08x siz 0x%08x add 0x%08x" % (buf.command, buf.size, buf.addr )
   return buf

# function used to transmit a message the to server and in most cases expect an answer back from the server 
# the message can be with or without a payload
# Andre fixme: for optimization it might be better to use call by reference instead of creating new buffers each time
def exchange( client_sock_file_desc, buf ) :
   # set the buffer size depending on payload (if any)
   if buf.command == PAYLOAD_WRITE :
      buf.size = buf.size # already set by the PAYLOAD_WRITE
   elif buf.command == SUM_SET or \
        buf.command == PAYLOAD_READ or \
        buf.command == MTI_QUIT :
      buf.size = SOCK_BUF_HEADER_SIZE + 4; # payload is one 32 bit word
   elif buf.command == MTI_FIRST_LOWER_REGION or \
        buf.command == MTI_NEXT_REGION or \
        buf.command == MTI_GET_LIBRARY_NAME or \
        buf.command == MTI_GET_PRIMARY_NAME or \
        buf.command == MTI_GET_REGION_NAME or \
        buf.command == MTI_GET_REGION_FULL_NAME or \
        buf.command == MTI_GET_REGION_SOURCE_NAME or \
        buf.command == MTI_GET_SECONDARY_NAME :
      buf.size = SOCK_BUF_HEADER_SIZE + 8 # payload is one 64 bit word
   elif buf.command == MTI_CMD or \
        buf.command == MTI_COMMAND or \
        buf.command == TRANSCRIPT_PRINT :
      # server expects the termination (null) character at the end of the string (payload will increase by one byte)
      buf.pl += struct.pack('B', 0)
      buf.size = SOCK_BUF_HEADER_SIZE + len(buf.pl)
      # print "buf.size ", buf.size
   elif buf.command == SEND_EMPTY_MESSAGE :
      buf.size = 0 # send no data
   else :
      buf.size = SOCK_BUF_HEADER_SIZE # only send the header
  
   # print "INFO    exchange: send %10d bytes, command %d" % (buf.size, buf.command)
   # print "data    tx cmd 0x%08x siz 0x%08x add 0x%08x" % (buf.command, buf.size, buf.addr )
   sock_transmit_data( client_sock_file_desc, buf )

   # there will be always a response from the server, except when an empty message (SEND_EMPTY_MESSAGE) is send to the server
   if buf.command != SEND_EMPTY_MESSAGE :
      buf = sock_receive_data( client_sock_file_desc )
      if buf.command == ERROR :
         print "ERROR   got ERROR response back from server"
      elif buf.command != OKAY :
         print "ERROR   got wrong %d response back from server" % buf.command
   return buf

def api_version_check( client_sock_file_desc, buf, verbose ) :
   buf.command = API_GET;
   buf = exchange( client_sock_file_desc, buf );

   (pl_u32_0,) = struct.unpack_from('<I', buf.pl, 0)

   if pl_u32_0 != API_VERSION :
      print "ERROR   server has %s api version, while client has %d api version" % (pl_u32_0, API_VERSION)
      sys.exit()

   if verbose != 0 :
      print "INFO    server API version %d" % pl_u32_0

# main
buf = sock_buf_class()
                  
optarg = 0 # Andre Fixme

client_sock_file_desc = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
client_sock_file_desc.connect("../socket/sock_file_handle")
api_version_check( client_sock_file_desc, buf, 1 )

# Note: the counter value is not from the vhdl, but from a c-model that replaced the vhdl
buf.command = COUNTER_GET
buf = exchange( client_sock_file_desc, buf )
print "INFO    current counter value %4d" % buf.pl_u32_0

# Note: the sqrt value is not from the vhdl, but from a c-model that replaced the vhdl
buf.command = SQRT_INT_GET
buf = exchange( client_sock_file_desc, buf )
print "INFO    current sqrt value %4d" % buf.pl_u32_0

buf.command = MTI_VERSION_GET
buf = exchange( client_sock_file_desc, buf )
print "INFO    mti version: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "date" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    date: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "examine top/d0/sqrt" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/d0/sqrt value: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "examine top/d0/cnt" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/d0/cnt value: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "examine top/d0/read_address" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/d0/read_address value: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "force top/rst TRUE" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/rst forced to TRUE: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "examine top/d0/sqrt" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/d0/sqrt value: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "examine top/d0/cnt" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/d0/cnt value: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "examine top/d0/read_address" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/d0/read_address value: %s" % buf.pl


buf.command = MTI_CMD
buf.pl = "noforce top/rst" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/rst released (no force): %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "examine top/d0/sqrt" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/d0/sqrt value: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "examine top/d0/cnt" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/d0/cnt value: %s" % buf.pl

buf.command = MTI_CMD
buf.pl = "examine top/d0/read_address" # use modelsim tcl to get a value
buf = exchange( client_sock_file_desc, buf ) 
print "INFO    top/d0/read_address value: %s" % buf.pl

buf.command = PAYLOAD_WRITE
buf.pl = ""
buf.size = SOCK_BUF_HEADER_SIZE + SOCK_BUF_PAYLOAD_SIZE
for ii in range( 0, SOCK_BUF_PAYLOAD_SIZE/4 ):
   buf.pl += struct.pack('I', ii)
buf = exchange( client_sock_file_desc, buf ) 

buf.command = PAYLOAD_READ
buf.pl = struct.pack('I', SOCK_BUF_PAYLOAD_SIZE)
buf.size = SOCK_BUF_HEADER_SIZE + 4
buf = exchange( client_sock_file_desc, buf ) 
for ii in range( 0, SOCK_BUF_PAYLOAD_SIZE/4 ):
   (read_data,) = struct.unpack_from('<I', buf.pl, ii*4)
   if read_data != ii :
      print "ERROR   read data %d expected %d" % (read_data, ii)


buf.command = TRANSCRIPT_PRINT
buf.pl = "all done at %s" % time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime())
buf = exchange( client_sock_file_desc, buf ) 

quit = True
quit = False
if quit :
   buf.command = MTI_QUIT
   buf.pl = struct.pack('I', MTI_QUIT_MAGIC)
   exchange( client_sock_file_desc, buf )

buf.command = DISCONNECT
buf = exchange( client_sock_file_desc, buf )

client_sock_file_desc.close()

print "all done"
