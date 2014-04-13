// Copyright 2012 - 2014 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h> // for basename

#include "sock_config.h"
#include "sock_functions.h"

// function used to transmit a message the to server and also expect a message back from the server
// the message can be with or without a payload
void exchange( int *client_sock_file_desc, sock_buf_t *buf )
{
   // set the buffer size depending on payload (if any), which can be a string or amount of words the message
   if( buf->command == PAYLOAD_WRITE ) 
   {
      // buf->size already set by the PAYLOAD_WRITE
   }
   else if( buf->command == SUM_SET || \
            buf->command == MTI_QUIT || \
            buf->command == PAYLOAD_READ )
   {
      buf->size = SOCK_BUF_HEADER_SIZE + 4; // payload is one 32 bit word
   }
   else if( buf->command == MTI_FIRST_LOWER_REGION || \
            buf->command == MTI_NEXT_REGION || \
            buf->command == MTI_GET_LIBRARY_NAME || \
            buf->command == MTI_GET_PRIMARY_NAME || \
            buf->command == MTI_GET_REGION_NAME || \
            buf->command == MTI_GET_REGION_FULL_NAME || \
            buf->command == MTI_GET_REGION_SOURCE_NAME || \
            buf->command == MTI_GET_SECONDARY_NAME )
   {
      buf->size = SOCK_BUF_HEADER_SIZE + 8; // payload is one 64 bit word
   }
   else if( buf->command == MTI_CMD || \
            buf->command == MTI_COMMAND || \
            buf->command == TRANSCRIPT_PRINT )
   {
      // also send the termination (null) character at the end of the string
      buf->size = SOCK_BUF_HEADER_SIZE + strlen( buf->pl.c8 ) + 1;
   }
   else if( buf->command == SEND_EMPTY_MESSAGE )
   {
      // send no data
      buf->size = 0;
   }
   else
   {
      // only send the header
      buf->size = SOCK_BUF_HEADER_SIZE;
   }
   
   // printf( "INFO    exchange: send %10d bytes, ", buf->size ); fflush(stdout);
   // printf( "command %d\n", buf->command );
   // printf( "data : 0x%08x 0x%08x 0x%08x\n", buf->command, buf->size, buf->addr );
   int bytes_transmitted = sock_transmit_data( client_sock_file_desc, buf );
   if( bytes_transmitted != buf->size )
   {
      printf( "ERROR   only %d bytes tranmitted instead of %d bytes, abort now\n", bytes_transmitted, buf->size );
      exit( EXIT_FAILURE );
   }

   // SEND_EMPTY_MESSAGE is the only normal situation when there is no message returned from server
   if( buf->command != SEND_EMPTY_MESSAGE )
   {
      int bytes_received = sock_receive_data( client_sock_file_desc, buf );
      if( bytes_received != buf->size )
      {
         printf( "ERROR   only %d bytes received instead of %d bytes, abord now\n", bytes_received, buf->size );
         exit( EXIT_FAILURE );
      }

      if( buf->command == ERROR )
      {
         printf( "ERROR   got ERROR response back from server\n" );
      }
      else if( buf->command != OKAY )
      {
         printf( "ERROR   got wrong %d response back from server\n", buf->command );
      }
   }
}

void data_test( int *client_sock_file_desc, sock_buf_t *buf )
{
   for( size_t ii = 0; ii < 3; ii++ )
   {
      buf->command = PAYLOAD_WRITE;
      buf->addr = 0;
      buf->size = SOCK_BUF_HEADER_SIZE + SOCK_BUF_PAYLOAD_SIZE;
      // fill buffer with stimuli
      int jj;
      for( jj = 0; jj < (SOCK_BUF_PAYLOAD_SIZE/4); jj++ )
      {
         buf->pl.u32[jj] = 0xdead0000 + jj + ii*10000;
      }
      exchange( client_sock_file_desc, buf );
   
      buf->command = PAYLOAD_READ;
      buf->addr = 0;
      buf->pl.u32[0] = SOCK_BUF_PAYLOAD_SIZE; // requested amount of bytes in first u32 payload
      buf->size = SOCK_BUF_HEADER_SIZE + 4;
      exchange( client_sock_file_desc, buf );
      // check return values from server
      for( jj = 0; jj < (SOCK_BUF_PAYLOAD_SIZE/4); jj++ )
      {
         if( buf->pl.u32[jj] != ( 0xdead0000 + jj + ii*10000 ) )
         {
            printf( "ERROR   difference between received 0x%08x and expected 0x%08x at index %d\n", buf->pl.u32[jj], (0xdead0000 + jj), jj );
         }
      }
   }
}

void wave_test( int *client_sock_file_desc, sock_buf_t *buf, unsigned long int iteration )
{
   buf->command = TIME_GET_RES;
   exchange( client_sock_file_desc, buf );
   int mti_res = (int) buf->pl.u32[0];
   char mti_res_text[16] = "undefined";
   if( mti_res == -6 ) { sprintf( mti_res_text, "us" ); }
   if( mti_res == -7 ) { sprintf( mti_res_text, "* 100 ns" ); }
   if( mti_res == -8 ) { sprintf( mti_res_text, "* 10 ns" ); }
   if( mti_res == -9 ) { sprintf( mti_res_text, "ns" ); }
   if( mti_res == -10 ) { sprintf( mti_res_text, "* 100 ps" ); }
   if( mti_res == -11 ) { sprintf( mti_res_text, "* 10 ps" ); }
   if( mti_res == -12 ) { sprintf( mti_res_text, "ps" ); } 

   buf->command = TIME_GET_NOW;
   exchange( client_sock_file_desc, buf );
   // convert time into readable format
   uint64_t mti_time = ((uint64_t) buf->pl.u32[1]<<32) + (uint64_t) buf->pl.u32[0];
   char time_string[32]; // used to store the thousands comma separated time string
   thousands_comma( time_string, 4, mti_time );
   printf( "INFO    iteration %12lu, current time %20s %s\n", iteration, time_string, mti_res_text );

   buf->command = MTI_CMD;
   sprintf( buf->pl.c8, "batch_mode" );
   exchange( client_sock_file_desc, buf );
   if( ( buf->pl.c8[0] != 49 ) && ( mti_time > 20000 ) ) // wave commands are not accepted in batch mode => ascii 49 = 1
   {
      // Warning: due to a bug in ModelSim/QuestaSim the following wave commands increases vish memory usage
      buf->command = MTI_CMD;
      sprintf( buf->pl.c8, "wave zoom range %llu %llu", (long long unsigned)(mti_time - 20000), (long long unsigned)mti_time );
      exchange( client_sock_file_desc, buf );
      if( buf->pl.c8[0] == 48 ) // returns string with left and right time => ascii 48 = 0
      {
         printf( "ERROR   wave zoom range command not accepted, return value '%s'\n", buf->pl.c8 ); 
      }

      buf->command = MTI_CMD;
      sprintf( buf->pl.c8, "wave cursor add -name \"Cursor 1\" -time %llu", (long long unsigned)(mti_time - 10000) );
      exchange( client_sock_file_desc, buf );
      if( buf->pl.c8[0] != 49 )  // returns cursor number => ascii 49 = 1
      { 
         printf( "ERROR   wave cursor add command not accepted, return value '%s'\n", buf->pl.c8 ); 
      }
   }
}

void pre_commands( int *client_sock_file_desc, sock_buf_t *buf )
{
   // this functions processes the commands that should be processed first
   printf( "INFO    buffer size %.3f Mbytes\n", (double) sizeof(sock_buf_t)/1000000.0 );

   buf->command = MTI_VERSION_GET;
   exchange( client_sock_file_desc, buf );
   printf( "INFO    mti version: '%s'\n", buf->pl.c8 );

   buf->command = COUNTER_GET;
   exchange( client_sock_file_desc, buf );
   printf( "INFO    current count value %4d", buf->pl.u32[0] );
   buf->command = SQRT_INT_GET;
   exchange( client_sock_file_desc, buf );
   printf( " current sqrt value %3d\n", buf->pl.u32[0] );

   buf->command = SUM_GET;
   exchange( client_sock_file_desc, buf );
   uint32_t sum = buf->pl.u32[0];
   printf( "INFO    get sum %u from server, ", sum );

   buf->command = SUM_SET;
   buf->pl.u32[0] = sum + 1;
   exchange( client_sock_file_desc, buf );

   buf->command = SUM_INC;
   exchange( client_sock_file_desc, buf );
      
   buf->command = SUM_GET;
   exchange( client_sock_file_desc, buf );
   printf( "after sum + 1 and inc, sum gets %u\n", buf->pl.u32[0] );

   buf->command = MTI_CMD;
   sprintf( buf->pl.c8, "date" );
   exchange( client_sock_file_desc, buf );
   printf( "INFO    mti cmd 'date' returns '%s'\n", buf->pl.c8 );
 
   if( 0 )
   {
      buf->command = MTI_CMD;
      sprintf( buf->pl.c8, "uptime" );
      exchange( client_sock_file_desc, buf );
      printf( "mti cmd return value for uptime '%s'\n", buf->pl.c8 );
   }
}

void mti_cmd( int *client_sock_file_desc, sock_buf_t *buf, char *mti_cmd_list, int verbose )
{
   buf->command = MTI_CMD;
   // also copy termination (null) character at the end of the string
   memcpy( buf->pl.c8, mti_cmd_list, ( 1 + strlen( mti_cmd_list ) ) );
   if( verbose != 0 )
   {
      printf( "INFO    mti cmd '%s'", buf->pl.c8 );
   }
   exchange( client_sock_file_desc, buf );
   if( verbose != 0 )
   {
      printf( " returns '%s'\n", buf->pl.c8 );
   }
}


uint64_t mti_get_top_region( int *client_sock_file_desc, sock_buf_t *buf )
{
   buf->command = MTI_GET_TOP_REGION;
   exchange( client_sock_file_desc, buf );
   return buf->pl.u64[0];
}

uint64_t mti_get_first_lower_region( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region )
{
   buf->command = MTI_FIRST_LOWER_REGION;
   buf->pl.u64[0] = region;
   exchange( client_sock_file_desc, buf );
   return buf->pl.u64[0];
}

uint64_t mti_get_next_region( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region )
{
   buf->command = MTI_NEXT_REGION;
   buf->pl.u64[0] = region;
   exchange( client_sock_file_desc, buf );
   return buf->pl.u64[0];
}

void mti_get_library_name( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region, char *return_value )
{
   buf->command = MTI_GET_LIBRARY_NAME;
   buf->pl.u64[0] = region;
   exchange( client_sock_file_desc, buf );
   strcpy( return_value, (basename(buf->pl.c8)) );
}

void mti_get_primary_name( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region, char *return_value )
{
   buf->command = MTI_GET_PRIMARY_NAME;
   buf->pl.u64[0] = region;
   exchange( client_sock_file_desc, buf );
   strcpy( return_value, buf->pl.c8 );
}

void mti_get_region_name( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region, char *return_value )
{
   buf->command = MTI_GET_REGION_NAME;
   buf->pl.u64[0] = region;
   exchange( client_sock_file_desc, buf );
   strcpy( return_value, buf->pl.c8 );
}

void mti_get_region_full_name( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region, char *return_value )
{
   buf->command = MTI_GET_REGION_FULL_NAME;
   buf->pl.u64[0] = region;
   exchange( client_sock_file_desc, buf );
   strcpy( return_value, buf->pl.c8 );
}

void mti_get_region_source_name( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region, char *return_value )
{
   buf->command = MTI_GET_REGION_SOURCE_NAME;
   buf->pl.u64[0] = region;
   exchange( client_sock_file_desc, buf );
   strcpy( return_value, buf->pl.c8 );
}

void mti_get_secondary_name( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region, char *return_value )
{
   buf->command = MTI_GET_SECONDARY_NAME;
   buf->pl.u64[0] = region;
   exchange( client_sock_file_desc, buf );
   strcpy( return_value, buf->pl.c8 );
}

void print_region( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region, size_t indent )
{
   char library_name[255];
   mti_get_library_name( client_sock_file_desc, buf, region, library_name );
   char region_name[255];
   mti_get_region_name( client_sock_file_desc, buf, region, region_name );
   char primary_name[255];
   mti_get_primary_name( client_sock_file_desc, buf, region, primary_name );
   char secondary_name[255];
   mti_get_secondary_name( client_sock_file_desc, buf, region, secondary_name );
   char source_name[255];
   mti_get_region_source_name( client_sock_file_desc, buf, region, source_name );
   printf( "INFO    " );
   printf( " %-15s %-10s ", source_name, library_name );
   for( size_t ii = 0; ii < 10; ii++ ) { if ( ii == indent ) { printf( "*" ); } else { printf( " " ); } }
   printf(" %-10s %-15s %-s\n", region_name, primary_name, secondary_name );
}

void hierarchy_recusive( int *client_sock_file_desc, sock_buf_t *buf, uint64_t region, size_t indent )
{
   if( region )
   {
      print_region( client_sock_file_desc, buf, region, indent );
      indent += 2;
      
      for( region = mti_get_first_lower_region( client_sock_file_desc, buf, region ); region; region = mti_get_next_region( client_sock_file_desc, buf, region ) )
      {
         hierarchy_recusive( client_sock_file_desc, buf, region, indent );
      }
   }
}

void hierarchy( int *client_sock_file_desc, sock_buf_t *buf )
{
   printf( "INFO    %-15s %-10s %-10s %-10s %-15s %-s\n", "File", "Library", "Hierarchy", "Instance", "Entity", "Archictecture" );
   // region represents a pointer, which is 64 bit on x86_64 and 32 bit on x86, so use always payload of 8 bytes
   uint64_t region = mti_get_top_region( client_sock_file_desc, buf );

   hierarchy_recusive( client_sock_file_desc, buf, region, 0 );
}  
 

void loop_test( sock_buf_t *buf, unsigned long int times, int waveform )
{
   unsigned long int ii;
   for( ii = 0; ii < times; ii++ )
   {
      int client_sock_file_desc = sock_client_setup( );
      sock_client_connect_to_server( client_sock_file_desc );

      // transmit data and control simulator (if requested and or possible)
      data_test( &client_sock_file_desc, buf );
      if( waveform == 1 )
      {
         wave_test( &client_sock_file_desc, buf, ii );
      }

      buf->command = DISCONNECT;
      exchange( &client_sock_file_desc, buf );
      close( client_sock_file_desc );
   }
   printf("INFO    client all done connecting %lu times to the server\n", ii );
}

void api_version_check( int *client_sock_file_desc, sock_buf_t *buf, int verbose )
{
   buf->command = API_GET;
   exchange( client_sock_file_desc, buf );

   if( buf->pl.u32[0] != API_VERSION )
   {
      printf( "ERROR   server has %d api version, while client has %d api version\n", buf->pl.u32[0], API_VERSION );
      exit( EXIT_FAILURE );
   }

   if( verbose != 0 )
   {
      printf( "INFO    server API version %d\n", buf->pl.u32[0] );
   }
}

void process_command( int *client_sock_file_desc, sock_buf_t *buf, int command, char *optarg, int verbose )
{
   buf->command = command;
   if( command == TRANSCRIPT_PRINT )
   {
      // also copy termination (null) character at the end of the string
      memcpy( buf->pl.c8, optarg, ( 1 + strlen( optarg ) ) );
   }
   else if( command == MTI_QUIT )
   {
      buf->pl.u32[0] = MTI_QUIT_MAGIC;
   }

   exchange( client_sock_file_desc, buf );
   if( buf->size > SOCK_BUF_HEADER_SIZE && verbose != 0 )
   {
      printf( "INFO    response '%s'\n", buf->pl.c8 );
   }
}

void sock_help( char *argv[] )
{
   printf( "Usage %s [OPTION]...\n", argv[0] );
   printf( "Communicate to simulator server thread through AF UNIX socket\n" );
   printf( "\n" );
   printf( "options:\n" );
   printf( "-b           send break to simulator\n" );
   printf( "-t           do some basic testing\n" );
   printf( "-q           quit simulator thread and server thread\n" );
   printf( "-l number    connect for n (unsigned long) times with server\n" );
   printf( "-m command   modelsim tcl command\n" );
   printf( "-p message   print messege on simulator console\n" );
   printf( "-v           print structure design\n" );
   printf( "-w           use waveform commands\n" );
   printf( "-h           display this help and exit\n" );
   printf( "with no options, the client will connect 10 times with the server\n" );
   printf( "Exit status %d if OK, %d if error\n", EXIT_SUCCESS, EXIT_FAILURE );
}

int main(int argc, char *argv[])
{
   sock_buf_t buf;
   int option = -1;

   // first process the options that do not require a connection
   while ((option = getopt (argc, argv, "bthl:m:p:qvw")) != -1)
   {
      switch (option)
      {
         case 'h':
             sock_help( argv );
             return EXIT_SUCCESS;
             break;
      } // switch case
   } // while

   // now process the options that require a connection with the server   
   optind = 1; // start re scanning of argv

   unsigned long times = 0;
   int sock_loop_test = 0;
   int waveform = 1;

   if( argc >= 2 )
   {
      // connect now with the server
      int client_sock_file_desc = sock_client_setup( );
      sock_client_connect_to_server( client_sock_file_desc );
      api_version_check( &client_sock_file_desc, &buf, 0 );
      while ((option = getopt (argc, argv, "bthl:m:p:qsvw")) != -1)
      {
         switch (option)
         {
            case 'b':
               process_command( &client_sock_file_desc, &buf, MTI_BREAK, optarg, 1 );
               break;
            case 't':
               api_version_check( &client_sock_file_desc, &buf, 1 );
               pre_commands( &client_sock_file_desc, &buf );
               break;
            case 'l':
               times = strtoul( optarg, NULL, 10 );
               sock_loop_test = 1;
               break;
            case 'm':
               mti_cmd( &client_sock_file_desc, &buf, optarg, 1 );
               break;
            case 'q':
               process_command( &client_sock_file_desc, &buf, MTI_QUIT, optarg, 1 );
               process_command( &client_sock_file_desc, &buf, DISCONNECT, optarg, 1 );
               close( client_sock_file_desc );
               return EXIT_SUCCESS;
               break;
            case 'p':
               process_command( &client_sock_file_desc, &buf, TRANSCRIPT_PRINT, optarg, 1 );
               break;
            case 'v':
               hierarchy( &client_sock_file_desc, &buf );
               break;
            case 'w':
               wave_test( &client_sock_file_desc, &buf, 0 );
               waveform = 1;
               break;
            default:
              break;
         } // switch case
      } // while

      process_command( &client_sock_file_desc, &buf, DISCONNECT, optarg, 1 );
      close( client_sock_file_desc );

      // the loop test takes care of opening and closing the socket by itself
      if( sock_loop_test == 1 )
      {
         loop_test( &buf, times, waveform );
      }

   } else {
      // no command options given, execute the default commands
      // connect now with the server
      int client_sock_file_desc = sock_client_setup( );
      sock_client_connect_to_server( client_sock_file_desc );
      api_version_check( &client_sock_file_desc, &buf, 1 );
      pre_commands( &client_sock_file_desc, &buf );
      hierarchy( &client_sock_file_desc, &buf );
      process_command( &client_sock_file_desc, &buf, DISCONNECT, optarg, 1 );
      close( client_sock_file_desc );

      loop_test( &buf, 10, 1 );
   } // if

   
   return EXIT_SUCCESS;
} // main
