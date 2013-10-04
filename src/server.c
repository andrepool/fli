// Copyright 2012, 2013 Andre Pool
// Licensed under the Apache License version 2.0
// you may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#include "global_var.h"
#include "sock_functions.h"
#include "server_functions.h"

#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// the server is started as a seperate thread
// the server functions as a bridge between the external control application and the globlal variable
// the global variable is used to exchange data between the different threads/functions

// this function makes use of the functions in the server_functions.c file
// one of the functions in that file takes care of the actual processing of the client commands

void *server( void *arg )
{
   printf( "INFO    server with API version %d\n", API_VERSION );
   // printf( "INFO    server global variable pointer %p, global variable size %.2fMByte\n", (void *) &gv, sizeof(gv)/(1024*1024.0) );

   gv.sum = 12;
   gv.handshake = 1; // enable handshake between client and testbench
 
   // create server socket which listens on file handle
   int server_sock_file_desc = sock_server_setup_and_listen( );

   printf( "INFO    wait for client connection\n" ); fflush(stdout);
   int shutdown = 0;
   unsigned int ii = 0;
   do
   {
      // this loop is entered each time the client (re)connects with the server
      // wait until the client is connected to the server socket
      int client_sock_file_desc = sock_server_wait_for_client( server_sock_file_desc );

      if( client_sock_file_desc > 0 )
      {
         if( ii % 0x10000 == 0 )
         {
            printf( "INFO    client connected to server for the 0x%08x time\n", ii ); fflush(stdout);
         }
         // the following function will return if one of the following events occurs:
         // client disconnect or client server shutdown (quit) request
         server_exchange_with_client( &client_sock_file_desc, &shutdown ); 
      }
      // the client has been disconnected, close the client file handle
      close( client_sock_file_desc );
      ii++;
   }
   while( shutdown == 0 );

   if( close( server_sock_file_desc ) < 0 )
   {
      printf( "WARNING errno message '%s', cannot close server socket\n", strerror( errno ) );
   }
   if( unlink(SOCK_FILE_HANDLE) < 0 )
   {
      printf( "WARNING errno message '%s', cannot unlink file handle\n", strerror( errno ) );
   }
   
   if( shutdown != 0 )
   {
      printf( "INFO    client has requisted to shutdown the server\n" ); fflush(stdout);
   }
   else
   {
      printf( "ERROR   unknown reason why server has been stopped\n" ); fflush(stdout);
   }
   
   return ( void* )( 1 );
}
