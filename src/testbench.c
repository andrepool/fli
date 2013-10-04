// Copyright 2012, 2013 Andre Pool
// Licensed under the Apache License version 2.0
// you may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

/*
This is the testbench c file that is connected to the testbench vhdl file using
the modelsim fli interface.

So from this c file the signals in the vhdl environment can be changed (and a
lot of other things).

This function can be triggered from signals in the testbench sensitivity list.

So when set correctly, the c file can generate a trigger to the vhdl world, and they
other way around.

In this testbench the goal is to use the c environment as leading, but unfortunately
the c world is not aware of "time" we generate the clock (time) in the vhdl world and
all other signals in the c world.

In vhdl the testbech is 1:1 connected to the dut.

Then the c level generates signals that are used by the dut, and the dut
generates signals that are analyzed and reactup upon by the c level.

The c level has a stimuli generator, a data analyzer and a global variable to exchange
information with other functions.

The stimuli generator and data analyzer are combined in one function.

The global variable is amongst others connected a server thread which makes it
possible to control the simulation environment from an external application.

The system runs as 2 (or more) threads:
1. simulator   - dut ( vhdl )
               - testbench ( c )
2. server
3 ..

The connection between the c testbench and vhdl simulator is synchronous

The connection between the server and c testbench is asynchronous

An client application is required to interface with the server to e.g.
control the testbench, simulator or readback status information from
testbench or simulator.

An example of some simulator control commands:
add wave signal_xx
quit
wave window full
*/

#include "mti.h"
#include "tcl.h"
#include "global_var.h"
#include "server.h"
#include "housekeeping.h"

#include <stdbool.h>
#include <string.h>

typedef struct
{
   mtiSignalIdT clk;
   mtiDriverIdT rst;
   mtiDriverIdT set;
   mtiDriverIdT inc;
   mtiDriverIdT dec;
   mtiDriverIdT load;
   mtiSignalIdT result;
   mtiSignalIdT cmp;
   mtiSignalIdT read_data;
} testbench_t;

static void testbench( void *param )
{
   _Bool clk, cmp;
   _Bool rst = false;
   _Bool set = false;
   mtiInt32T load = 0;
   mtiInt32T result, read_data;

   static unsigned long long clk_cnt = 0;
   static _Bool inc = false;
   static _Bool dec = false;
   
   testbench_t * ip = (testbench_t *) param;
   clk = mti_GetSignalValue ( ip->clk );
   cmp = mti_GetSignalValue ( ip->cmp );
   result = mti_GetSignalValue (ip->result );
   read_data = mti_GetSignalValue (ip->read_data );
  
   if( clk )
   {
      clk_cnt++;
   }
      
   if( clk_cnt >=3 && clk_cnt <= 7 )
   {
      rst = true;
   }
    
   if( clk_cnt == 9 )
   {
      set = true;
      load = 27;
   }
     
   if( clk_cnt == 12 )
   {
      inc = true;
   }

   if( result > 19 || result < 1 )
   {
      /* in case a force is used and the inc or dec are still active the value can go out of our "visible" test range */
      set = true;
      load = 27;
   }
   else if( result > 18 )
   {
      inc = false;
      dec = true;
   }
   else if( result < 2 )
   {
      inc = true;
      dec = false;
   }
  
   mti_ScheduleDriver( ip->rst, rst, 2, MTI_INERTIAL );
   mti_ScheduleDriver( ip->set, set, 1, MTI_INERTIAL );
   mti_ScheduleDriver( ip->inc, inc, 1, MTI_INERTIAL );
   mti_ScheduleDriver( ip->dec, dec, 1, MTI_INERTIAL );
   mti_ScheduleDriver( ip->load, load, 2, MTI_INERTIAL );
     
#ifdef NONO
   printf( "testbech clk_cnt %llu clk %u, rst %u, set %u, inc %u, dec %u, load %d, result %d, cmp %u\n",
      clk_cnt, clk, rst, set, inc, dec, load, result, cmp );
   fflush(stdout);
#endif

}

/* this function is called once at startup and configures testbench interface */
void testbench_init(
   mtiRegionIdT region,
   char *param,
   mtiInterfaceListT *generics,
   mtiInterfaceListT *ports
)
{  
   // printf( "INFO    testbench global variable pointer %p, global variable size %.2fMByte\n", (void *) &gv, sizeof(gv)/(1024*1024.0) );
   
   /* connect the vhdl inputs and outputs to the ip struct */
   testbench_t *ip;
   ip = (testbench_t *) mti_Malloc( sizeof( testbench_t) );
   ip->clk = mti_FindPort( ports, "clk" );
   ip->rst = mti_CreateDriver( mti_FindPort( ports, "rst" ) );
   ip->set = mti_CreateDriver( mti_FindPort( ports, "set" ) );
   ip->inc = mti_CreateDriver( mti_FindPort( ports, "inc" ) );
   ip->dec = mti_CreateDriver( mti_FindPort( ports, "dec" ) );
   ip->load = mti_CreateDriver( mti_FindPort( ports, "load" ) );
   
   ip->result = mti_FindPort( ports, "result" );
   ip->cmp = mti_FindPort( ports, "cmp" );
   ip->read_data = mti_FindPort( ports, "read_data" );
   
   mtiProcessIdT testbench_process = mti_CreateProcess( "testbench_p", testbench, ip );
   mti_Sensitize( testbench_process, ip->clk, MTI_EVENT);

   /* start also a housekeeping process that handles non testbench related issues */
   mtiProcessIdT housekeeping_process = mti_CreateProcess( "housekeeping_p", housekeeping, ip );
   mti_Sensitize( housekeeping_process, ip->clk, MTI_EVENT);

   /* start the thread which allows us to connect on the socket */
   pthread_t socket_thread;
   pthread_mutex_init(&gv_mutex_lock, NULL);
   int return_code =  pthread_create( &socket_thread, NULL, server, NULL );
   if( return_code != 0 )
   {
      printf( "ERROR: return code thread 0 with %d\n", return_code );
   }
   fflush(stdout);   
}


