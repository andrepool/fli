// Copyright 2012, 2013 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#include "mti.h"
#include "tcl.h"
#include "global_var.h"

#include <string.h>
#include <sys/time.h>

void housekeeping( void *param )
{
   if( gv.buf_valid != 0 )
   {
      // printf( "INFO    housekeeping global variable pointer %p, global variable size %.2fMByte\n", (void *) &gv, sizeof(gv)/(1024*1024.0) );
      // printf( "INFO    housekeeping received command %d\n", gv.receive.command );
      if( gv.receive.command == MTI_PROC_TEST1 )
      {
         gv_valid( 0 ); // command is accepted by this process
         printf( "mti proc test1\n");
         sprintf( gv.transmit.pl.c8, "mti proc test1 string" );
         gv.transmit.size =  SOCK_BUF_HEADER_SIZE + strlen( gv.transmit.pl.c8 ) + 1; // +1 -> termination character
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_BREAK )
      {
         gv_valid( 0 ); // command is accepted by this process
         mti_Break( );
         sprintf( gv.transmit.pl.c8, "mti break has been accepted by simulator" );
         gv.transmit.size =  SOCK_BUF_HEADER_SIZE + strlen( gv.transmit.pl.c8 ) + 1; // +1 -> termination character
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == TRANSCRIPT_PRINT )
      {
         gv_valid( 0 ); // command is accepted by this process
         printf( "%s\n", gv.receive.pl.c8 ); fflush(stdout);
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_VERSION_GET )
      {
         gv_valid( 0 ); // command is accepted by this process
         strcpy( gv.transmit.pl.c8, mti_GetProductVersion( ) );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == TIME_GET_NOW )
      {
         gv_valid( 0 ); // command is accepted by this process
         gv.transmit.pl.u32[0] = (uint32_t) mti_Now( );
         gv.transmit.pl.u32[1] = (uint32_t) mti_NowUpper( );
         gv.transmit.size += 8;
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == TIME_GET_RES )
      {
         gv_valid( 0 ); // command is accepted by this process
         gv.transmit.pl.u32[0] = (uint32_t) mti_GetResolutionLimit( );
         gv.transmit.size += 4;
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_CMD )
      {
         gv_valid( 0 ); // command is accepted by this process

         Tcl_Interp *interp;
         interp = mti_Interp();
         int status = mti_Cmd( gv.receive.pl.c8 );
         if( status != TCL_OK )
         {
            sprintf( gv.transmit.pl.c8, "executing MTI_CMD command returned %d instead of %d (TCL_OK)", status, TCL_OK );
            gv.transmit.size =  SOCK_BUF_HEADER_SIZE + strlen( gv.transmit.pl.c8 ) + 1; // +1 -> termination character
            gv.transmit.command = ERROR;
            printf( "ERROR   %s\n", gv.transmit.pl.c8 ); fflush(stdout);
         }
         else
         {
            // send mti_cmd result back to the server
            strcpy( gv.transmit.pl.c8, interp->result );
            gv.transmit.size =  SOCK_BUF_HEADER_SIZE + strlen( gv.transmit.pl.c8 ) + 1; // +1 -> termination character
            // echo the information in the transcript windows (which is not performed by mti_Cmd)
            // printf( "INFO     tcl: %s\n", gv.transmit.pl.c8 ); fflush(stdout);
         }
         Tcl_ResetResult( interp );
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_COMMAND )
      {
         gv_valid( 0 ); // command is accepted by this process

         mti_Command( gv.receive.pl.c8 ); // response of mti_command in vsim transcript window
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_FIRST_LOWER_REGION )
      {
         gv_valid( 0 ); // command is accepted by this process
         gv.transmit.pl.u64[0] = (unsigned long int) mti_FirstLowerRegion( (mtiRegionIdT)(unsigned long int) gv.receive.pl.u64[0] );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + 8;
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_GET_LIBRARY_NAME )
      {
         gv_valid( 0 ); // command is accepted by this process
         strcpy( gv.transmit.pl.c8, mti_GetLibraryName( (mtiRegionIdT)(unsigned long int) gv.receive.pl.u64[0] ) );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_GET_PRIMARY_NAME )
      {
         gv_valid( 0 ); // command is accepted by this process
         strcpy( gv.transmit.pl.c8, mti_GetPrimaryName( (mtiRegionIdT)(unsigned long int) gv.receive.pl.u64[0] ) );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_GET_REGION_NAME )
      {
         // instantiation name
         gv_valid( 0 ); // command is accepted by this process
         strcpy( gv.transmit.pl.c8, mti_GetRegionName( (mtiRegionIdT)(unsigned long int) gv.receive.pl.u64[0] ) );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_GET_REGION_FULL_NAME )
      {
         gv_valid( 0 ); // command is accepted by this process
         strcpy( gv.transmit.pl.c8, mti_GetRegionFullName( (mtiRegionIdT)(unsigned long int) gv.receive.pl.u64[0] ) );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_GET_REGION_SOURCE_NAME )
      {
         // architecture name
         gv_valid( 0 ); // command is accepted by this process
         strcpy( gv.transmit.pl.c8, mti_GetRegionSourceName( (mtiRegionIdT)(unsigned long int) gv.receive.pl.u64[0] ) );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_GET_SECONDARY_NAME )
      {
         // file name of the instantiation
         gv_valid( 0 ); // command is accepted by this process
         strcpy( gv.transmit.pl.c8, mti_GetSecondaryName( (mtiRegionIdT)(unsigned long int) gv.receive.pl.u64[0] ) );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + strlen(gv.transmit.pl.c8) + 1; // +1 -> termination character
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_GET_TOP_REGION )
      {
         gv_valid( 0 ); // command is accepted by this process
         gv.transmit.pl.u64[0] = (unsigned long int) mti_GetTopRegion( );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + 8;
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_NEXT_REGION )
      {
         gv_valid( 0 ); // command is accepted by this process
         gv.transmit.pl.u64[0] = (unsigned long int) mti_NextRegion( (mtiRegionIdT)(unsigned long int) gv.receive.pl.u64[0] );
         gv.transmit.size = SOCK_BUF_HEADER_SIZE + 8;
         gv_acknowledge( 1 ); // response of this process is ready
      }
      else if( gv.receive.command == MTI_QUIT )
      {
         gv_valid( 0 );
         if( gv.receive.pl.u32[0] == MTI_QUIT_MAGIC )
         {
             printf( "WARNING simulator quit requested by configuration application\n"); fflush(stdout);
             sprintf( gv.transmit.pl.c8, "simulator has stopped" );
             gv.transmit.size =  SOCK_BUF_HEADER_SIZE + strlen( gv.transmit.pl.c8 ) + 1; // +1 -> termination character
             gv.transmit.command = SHUTDOWN;
         }
         else
         {
            sprintf( gv.transmit.pl.c8, "ERROR   simulator quit request has wrong magic 0x%08x instead of 0x%08x", gv.receive.pl.u32[0], MTI_QUIT_MAGIC );
            gv.transmit.size =  SOCK_BUF_HEADER_SIZE + strlen( gv.transmit.pl.c8 ) + 1; // +1 -> termination character
            gv.transmit.command = ERROR;
            printf( "%s\n", gv.transmit.pl.c8 ); fflush(stdout);
         }
         gv_acknowledge( 1 ); // response of this process is ready

         if( gv.receive.pl.u32[0] == MTI_QUIT_MAGIC )
         {
            // we want to be able to show the quit message in the simulator window and using the clean
            // disconnect function of the client
            // unfortunately the sleep function does not work so use the gettimeofday instead
            struct timeval tv;
            gettimeofday(&tv, NULL); 
            long current_time = tv.tv_sec;
            long stop_time = current_time + 2; // use at least +2 = > 1 second so the client has time to execute the disconnect
            while ( current_time < stop_time )
            {
               gettimeofday(&tv, NULL);
               current_time = tv.tv_sec;
            }
            mti_Quit( );
         }
      }
   }
}
