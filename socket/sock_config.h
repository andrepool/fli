// Copyright 2012 - 2015 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#ifndef INCLUDED_SOCK_CONFIG_H
#define INCLUDED_SOCK_CONFIG_H

#include <stdint.h>
// pthread.h include required for ptread_t
#include <pthread.h>

#define API_VERSION 13

#define USE_AF_INIT
#ifdef USE_AF_INIT
#define TCP_IP_PORT 2222
#define TCP_IP_ADDRESS "127.0.0.1"
#else
#define SOCK_FILE_HANDLE "../socket/sock_file_handle"
#endif


// Command list and response list.
// This list is used by client to send commands to server, and by server to send back
// responses to client.
// The list is subject to change, however the position of the first two elements may
// not be changed to garantee that the correct API version can be resolved.
// When the list is changed, the API_VERSION number should be increased.
typedef enum command_response_list {
   AA_DO_NOT_USE_ZERO = 0,
   API_GET,
   COUNTER_GET,
   DISCONNECT, // sending disconnect is same as transmitting 0 bytes from the client to server
   ERROR,
   ERROR_MESSAGE_SIZE,
   MTI_BREAK,
   MTI_CMD, // result not transcribed in transcript window but returned to control application
   MTI_COMMAND, // result in vsim transcript window
   MTI_FIRST_LOWER_REGION,
   MTI_GET_LIBRARY_NAME,
   MTI_GET_PRIMARY_NAME, // entity, package or module name
   MTI_GET_REGION_NAME, // instantiation
   MTI_GET_REGION_FULL_NAME, // hierarchical name of instantiation
   MTI_GET_REGION_SOURCE_NAME, // source file which contains instantiation
   MTI_GET_SECONDARY_NAME, // architecture name
   MTI_GET_TOP_REGION,
   MTI_NEXT_REGION,
   MTI_PROC_TEST1,
   MTI_QUIT,
   MTI_VERSION_GET,
   OKAY,
   PAYLOAD_READ,
   PAYLOAD_WRITE,
   SEND_EMPTY_MESSAGE, // sending 0 bytes, same server behaviour as disconnect, only for testingsame as disconnect, but only testing
   SHUTDOWN,
   SQRT_INT_GET,
   SUM_INC,
   SUM_GET,
   SUM_SET,
   TIME_GET_NOW,
   TIME_GET_RES,
   TRANSCRIPT_PRINT,
   ZZ_DO_NOT_USE_LAST
} command_response_t;

// The simulator can only be stopped when the command and the magic are correct.
#define MTI_QUIT_MAGIC 0xabcd0011

// A sock_buf_t struct is used to assemble a message to and from the server.
// The struct message exists of a header and payload.
// To make interfacing with e.g. scripting easier, the header and payload are 32 bit alligned.

// The header exists of:
// - command, which also can represent a response
// - size in bytes of the message (header + payload)
// - addr used when a specific (start) address is required probably used together with
//   the read and write command, for write the amount of bytes = size - header, for
//   read the amount of bytes is requested by the first payload word u32[0]
// - reserved field

// The payload is optional, so the minimal size of the message is the size of the header.
// The struct allows payload access through different data types, saving the need for
// casting and offset calculation.

// payload size in bytes, for 32 bit allignment the size should be a multiple of 4
// 1GB buffer (server = buffer + global memory = 2GB) and 1GB client = 3GB memory does not work with simulator
// #define SOCK_BUF_PAYLOAD_SIZE ( 0x40000000 )
// 512MB buffer (server = buffer + global memory = 1.5GB) and 512MB client = 2GB memory seems to work
// #define SOCK_BUF_PAYLOAD_SIZE ( 0x20000000 )
// 512kB buffer (server = buffer + global memory = 1.5MB) and 512kB client = 2MB
#define SOCK_BUF_PAYLOAD_SIZE ( 0x00080000 )

// for efficient payload allignment on x86-64 the header should be a multiple of 64 bits
typedef struct
{
   uint32_t command; // command or response
   uint32_t size; // message size in bytes
   uint32_t addr;
   uint32_t reserved;
   union
   {
      char c8[SOCK_BUF_PAYLOAD_SIZE]; // payload of char type (for gcc this is signed)
      uint8_t u8[SOCK_BUF_PAYLOAD_SIZE]; // payload of u8 type
      uint16_t u16[(SOCK_BUF_PAYLOAD_SIZE + 1)/2]; // payload of u8 type
      uint32_t u32[(SOCK_BUF_PAYLOAD_SIZE + 3)/4]; // payload of u32 type
      uint64_t u64[(SOCK_BUF_PAYLOAD_SIZE + 7)/8]; // payload of u64 type
   } pl;
} sock_buf_t;
// } __attribute__((packed)) sock_buf_t; // results in smaller, but probably slower struct if types cannot be alligned on system type

#define SOCK_BUF_HEADER_SIZE ( sizeof(sock_buf_t) - SOCK_BUF_PAYLOAD_SIZE )

#endif /* INCLUDED_SOCK_CONFIG_H */
