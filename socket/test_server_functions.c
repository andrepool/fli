// Copyright 2014 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#include "mti.h"
#include "server_functions.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_server_provide_data( )
{
   if( gv.receive.command == MTI_VERSION_GET )
   {
      gv_valid( 0 ); // command is accepted by this process
      sprintf( gv.transmit.pl.c8, "test version 1.0" );
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == TRANSCRIPT_PRINT )
   {
      gv_valid( 0 ); // command is accepted by this process
      printf( "%s\n", gv.receive.pl.c8 ); fflush(stdout);
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == TIME_GET_NOW )
   {
      gv_valid( 0 ); // command is accepted by this process
      gv.transmit.pl.u32[0] = 123456789;
      gv.transmit.pl.u32[1] = 0;
      gv.transmit.size += 8;
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == TIME_GET_RES )
   {
      gv_valid( 0 ); // command is accepted by this process
      gv.transmit.pl.u32[0] = -6;
      gv.transmit.size += 4;
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == MTI_CMD ||
            gv.receive.command == MTI_COMMAND )
   {
      gv_valid( 0 ); // command is accepted by this process
      sprintf( gv.transmit.pl.c8, "test cmd" );
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == MTI_GET_LIBRARY_NAME )
   {
      gv_valid( 0 ); // command is accepted by this process
      sprintf( gv.transmit.pl.c8, "test library" );
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == MTI_GET_PRIMARY_NAME )
   {
      gv_valid( 0 ); // command is accepted by this process
      sprintf( gv.transmit.pl.c8, "test primary library" );
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == MTI_GET_REGION_NAME )
   {
      // instantiation name
      gv_valid( 0 ); // command is accepted by this process
      sprintf( gv.transmit.pl.c8, "test region" );
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == MTI_GET_REGION_FULL_NAME )
   {
      gv_valid( 0 ); // command is accepted by this process
      sprintf( gv.transmit.pl.c8, "test full region" );
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == MTI_GET_REGION_SOURCE_NAME )
   {
      // architecture name
      gv_valid( 0 ); // command is accepted by this process
      sprintf( gv.transmit.pl.c8, "test architecture" );
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == MTI_GET_SECONDARY_NAME )
   {
      // file name of the instantiation
      gv_valid( 0 ); // command is accepted by this process
      sprintf( gv.transmit.pl.c8, "test instance" );
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
      gv_acknowledge( 1 ); // response of this process is ready
   }
   else if( gv.receive.command == MTI_QUIT )
   {
      // file name of the instantiation
      gv_valid( 0 ); // command is accepted by this process
      sprintf( gv.transmit.pl.c8, "test quit" );
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
      gv_acknowledge( 1 ); // response of this process is ready
      exit( EXIT_SUCCESS );
   }
   else
   {
      gv_valid( 0 ); // command is accepted by this process
      gv.transmit.pl.u64[0] = 0;
      gv.transmit.size = SOCK_BUF_HEADER_SIZE + 8;
      gv_acknowledge( 1 ); // response of this process is ready
   }
}

