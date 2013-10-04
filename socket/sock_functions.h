// Copyright 2012, 2013 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

/* Generic used functions for a client and server which communicate 
using a stream socket (TCP) in the Unix domain */


#ifndef INCLUDED_SOCK_FUNCTIONS_H
#define INCLUDED_SOCK_FUNCTIONS_H

// sock_config.h required for sock_buf_t type
#include "sock_config.h"
#include <sys/types.h>

// create server socket which listens on file handle
// returns file descriptor
int sock_server_setup_and_listen( );

// create client socket which opens a file file handle
// returns file descriptor
int sock_client_setup( );

// use client file descriptor to connect to server
void sock_client_connect_to_server( int client_sock_file_desc );

// wait until client is connected to server socket
// returns file descriptor
int sock_server_wait_for_client(  int server_sock_file_desc );

// receive data from socket
// returns amount of bytes received, -1 in case of an error or 0 for disconnect
ssize_t sock_receive_data ( int *client_sock_file_desc, sock_buf_t *buf );

// transmit data to socket
// returns amount of bytes transmitted, -1 in case of an error or 0 for disconnect
int sock_transmit_data( int *client_sock_file_desc, sock_buf_t *buf );

// the print  %'d is not supported by ansi c, the following function creates
// a string with the thousand comma separator from a number
void thousands_comma( char *strptr, size_t order, uint64_t number );

#endif /* INCLUDED_SOCK_FUNCTIONS_H */
