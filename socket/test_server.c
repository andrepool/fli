// Copyright 2012, 2013 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

/*
Server application thats sets up a socket, waits for a client
to connect, handles the message from the client and return the
appropriate message back to the client.

The system make use of the Unix Socket type.

The purpose is to show a basic server that can be used in conjunction
with modeltech modelsim/questasim fli interface, to allow us to simulate
the dut in vhdl/verilog/systemverilog and the testbench and other time
consuming blocks in c and using this server to interface with the c part
of the simulation environment.
*/

#include "global_var.h"
#include "sock_functions.h"
#include "server_functions.h"

#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main( )
{
   printf( "INFO    server API version %d\n", API_VERSION);
   printf( "INFO    server global variable pointer %p, global variable size %.2fMByte\n", (void *) &gv, sizeof(gv)/(1024*1024.0) );

   gv.sum = 15;
   gv.handshake = 0; // test_server has no testbench, so do not wait for the testbench handshake
   pthread_mutex_init(&gv_mutex_lock, NULL);
   
   // create server socket which listens on file handle
   int server_sock_file_desc = sock_server_setup_and_listen( );
    
   printf("INFO    wait for client connection\n"); fflush(stdout);
   unsigned int ii = 0;
   // the server will run until ctrl-c
   while( 1 )
   {
      // this loop is entered each time the client (re)connects with the server
      // wait until the client is connected to the server socket

      int client_sock_file_desc = sock_server_wait_for_client( server_sock_file_desc );
      
      if( client_sock_file_desc > 0 )
      {
         if( ii % 0x10000 == 0 )
         {
            printf( "INFO    client connected to server for the 0x%08x time\n", ii );
         }
         server_exchange_with_client( &client_sock_file_desc ); 
         // at this moment the client has been disconnected
      }
      // the current file handle is not used by the client anymore
      close( client_sock_file_desc );
      ii++;
   }

   if( close( server_sock_file_desc ) < 0 )
   {
      printf( "WARNING errno message '%s', cannot close server socket\n", strerror( errno ) );
   }
   if( unlink(SOCK_FILE_HANDLE) < 0 )
   {
      printf( "WARNING errno message '%s', cannot unlink file handle\n", strerror( errno ) );
   }

   return 0;
}
