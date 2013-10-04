// Copyright 2012, 2013 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#define _POSIX_C_SOURCE	199309L

#include "mti.h"
#include "server_functions.h"
#include "sock_functions.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

/* Process one command from the receive buffer and create appropriate message in transmit buffer.
The receive buffer is filled by the client and the transmit buffer will be send to the client.
The message can be with or without payload.
*/
void process_command_from_client( int *disconnect, int *shutdown )
{
   int ii = 0;
  
   // set defaults response and buffer size, only a few cases need other values
   gv.transmit.size = SOCK_BUF_HEADER_SIZE;
   gv.transmit.data = 0;
   gv.transmit.command = OKAY;
   if( gv.receive.command == API_GET )
   {
      gv.transmit.data = API_VERSION;
   }
   else if( gv.receive.command == DISCONNECT )
   {
      *disconnect = 1;
   }   
   else if( gv.receive.command == SERVER_SHUTDOWN )
   {
      if( gv.receive.data == SERVER_SHUTDOWN_MAGIC )
      {
         *shutdown = 1;
         sprintf( gv.transmit.pl.c8, "server shutdown requisted by control application" );
         gv.transmit.size =  SOCK_BUF_HEADER_SIZE + strlen( gv.transmit.pl.c8 ) + 1; // +1 -> termination character
         printf( "INFO    %s\n", gv.transmit.pl.c8 );
      }
      else
      {
         sprintf( gv.transmit.pl.c8, "client tried to shutdown server, but used wrong 0x%08x shutdown value", gv.receive.data );
         gv.transmit.size =  SOCK_BUF_HEADER_SIZE + strlen( gv.transmit.pl.c8 ) + 1; // +1 -> termination character
         gv.transmit.command = ERROR;
         printf( "ERROR   %s\n", gv.transmit.pl.c8 );
         fflush(stdout);
         *disconnect = 1;
      }
   }   
   else if( gv.receive.command == SUM_GET )
   {
      gv.transmit.data = gv.sum;
   }
   else if( gv.receive.command == SUM_INC )
   {
      printf("INFO    incr sum from %d to ", gv.sum );
      pthread_mutex_lock( &gv_mutex_lock );
      gv.sum++;
      pthread_mutex_unlock( &gv_mutex_lock );
      printf("%d\n", gv.sum );
   }
   else if( gv.receive.command == SUM_SET )
   {
      printf("INFO    set sum from %d to ", gv.sum );
      pthread_mutex_lock( &gv_mutex_lock );
      gv.sum = gv.receive.data;
      pthread_mutex_unlock( &gv_mutex_lock );
      printf("%d\n", gv.sum );
   }
   else if( gv.receive.command == PAYLOAD_WRITE )
   {
      // printf( "INFO    receive  %d words from client\n", gv.receive.words );
      pthread_mutex_lock( &gv_mutex_lock );
      for( ii = 0; ii < gv.receive.words; ii++ )
      {
         // PAYLOAD_WRITE data in global memory buffer
         gv.memory[(gv.receive.addr) + ii] = gv.receive.pl.u32[ii];
      }
      pthread_mutex_unlock( &gv_mutex_lock );
   }
   else if( gv.receive.command == PAYLOAD_READ )
   {
      for( ii = 0; ii < gv.receive.words; ii++ )
      {
         // read data from global memory buffer
         gv.transmit.pl.u32[ii] = gv.memory[(gv.receive.addr) + ii];
      }
      gv.transmit.words = gv.receive.words; // same amount of words as requested
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + (gv.receive.words<<2);
      // printf( "INFO    transmit %d words to client\n", gv.receive.words );
   }
   else
   {
      // this command is unknown by server, so maybe it is known by one of the modelsim processes
      // ask all modelsim processes to have a look and if they now how to process they will respond with a message
      // if after a while nobody responds, display an error and reject command
      
      gv_acknowledge( 0 ); // clear the previous acknowledge flag from one of the modelsim processes (if any)
      gv_valid( 1 );// inform the modelsim processes a command is waiting to be processed

      // now wait for the response from a modelsim process
      int print_warning = 0;
      while( gv.buf_acknowledge == 0 && gv.handshake == 1 && print_warning < 5000 )
      {
         if( print_warning % 1000 == 999 )
         {
            printf("WARNING command waiting in buffer to be processed by ModelSim process\n");
         }
         print_warning++;
         nanosleep((struct timespec[]){{0, 5*1000000}}, NULL); // 5 ms
      }
      if( gv.buf_acknowledge == 0 && gv.handshake == 1 )
      {
         printf("ERROR   no ModelSim process responded on command %d in buffer with message\n", gv.receive.command );
         printf("ERROR   %s\n", gv.receive.pl.c8 );
         fflush( stdout );
         gv.transmit.command = ERROR;
      }
      else
      {
         // printf( "INFO    a ModelSim process returned status %d\n", gv.transmit.command );
         if( gv.transmit.size > SOCK_BUF_HEADER_SIZE )
         {
            // printf( "INFO    message: %s\n", gv.transmit.pl.c8 );
         }
      }

      // in case the quit command was requested, which will be processed by the housekeeping process,
      // the server can be shutdown
      if( ( gv.receive.command == MTI_QUIT ) && ( gv.receive.data == MTI_QUIT_MAGIC ) ) {  *shutdown = 1; }
   }
}

/* 
Client is connected to server, now we can receive messages from client,
handle the data from client (process commands), create responses, and send
these responses back to client.
Only when the client is disconnected, this function will return, otherwise
this function will wait for the following command and respond with the appropriate
message.
 */
void server_exchange_with_client( int *client_sock_file_desc, int *shutdown )
{
   // create buffer which we can use to receive and transmit data
   int disconnect = 0;
   do
   {
      ssize_t bytes_received = sock_receive_data ( client_sock_file_desc, &gv.receive );
      // there are 4 return value situations: error, disconnect, invalid size and correct
      
      if( bytes_received < 0 )
      {
         // the error has already be printed in sock_receive_data function
         // this is a quite serious error, abort the connection with the client
         disconnect = 1;
      }
      else if( bytes_received == 0 )
      {
         // the client is unexpected disconnected, their is no connection anymore so no return message
         printf( "WARNING client was gone without sending a disconnect first\n" );
         fflush( stdout );
         disconnect = 1;
      }
      else
      {
         if( ( bytes_received != (ssize_t) gv.receive.size ) && ( bytes_received != 0 ) )
         {
            printf( "ERROR   got only %lld bytes instead of %d bytes, probably error in byte size calculation, command %d\n", (long long int)bytes_received, gv.receive.size, gv.receive.command );
            printf( "ERROR   ignore this command and inform the client about this error\n" );
            fflush( stdout );
            gv.transmit.command = ERROR_MESSAGE_SIZE;
            gv.transmit.size = SOCK_BUF_HEADER_SIZE;
         }
         else
         {
            // a "valid" message has been received from client, now process the message
            // printf( "recieved %d bytes\n", gv.receive.size );
            process_command_from_client( &disconnect, shutdown );
            // printf("transmitted %d bytes\n", gv.receive.size );
         }
         // send the appropriate message back to the client
         int bytes_transmitted = sock_transmit_data( client_sock_file_desc, &gv.transmit );

         if( bytes_transmitted < 0 )
         {
            // the error has already be printed in sock_transmit_data function
            // this is a quite serious error, abort the connection with the client
            disconnect = 1;
         }
         else if( bytes_transmitted != gv.transmit.size )
         { 
            printf( "ERROR   only transmitted %d bytes instead of %d bytes\n", bytes_transmitted, gv.transmit.size );
            printf( "ERROR   abort connection (if any) with client\n" );
            fflush( stdout );
            disconnect = 1;
         }
      }
   }
   // we keep on looping until a shutdown or disconnect is requisted or an error occurred in which
   // case we also disconnect with the client and wait for a new connection
   while( ( *shutdown == 0 ) && (disconnect == 0 ) );
}
