// Copyright 2012, 2013 Andre Pool
// Licensed under the Apache License version 2.0
// you may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#include "mti.h"
#include "global_var.h"
#include <stdio.h>

// create one struct that contains all vhdl signals that
// need to be passed to the function
typedef struct
{
   mtiSignalIdT clk;
   mtiSignalIdT rst;
   mtiSignalIdT set;
   mtiSignalIdT inc;
   mtiSignalIdT dec;
   mtiSignalIdT load;
   mtiDriverIdT cnt;
} counter_t;

// the process function that will be called on each event
// (in this case only clk)
static void counter( void *param )
{
   // connect function argument to counter struct
   counter_t * ip = (counter_t *) param;

   // get current values from the vhdl world
   _Bool clk = (_Bool) mti_GetSignalValue ( ip->clk );
   _Bool rst = (_Bool) mti_GetSignalValue ( ip->rst );
   _Bool set = (_Bool) mti_GetSignalValue ( ip->set );
   _Bool inc = (_Bool) mti_GetSignalValue ( ip->inc );
   _Bool dec = (_Bool) mti_GetSignalValue ( ip->dec );
   mtiInt32T load = mti_GetSignalValue (ip->load );
 
   // implement the counter functionality
   static mtiInt32T cnt = 7; // initial value
   if( clk )
   {
      if( rst )
      {
         cnt = 0;
      }
      else if( set )
      {
         cnt = load;
      }
      else if( inc )
      {
         cnt++;
      }
      else if( dec )
      {
         cnt--;
      }
   }
   
   // send value cnt back to vhdl world with 1 ns delay
   mti_ScheduleDriver( ip->cnt, cnt, 1, MTI_INERTIAL );

   // process commands from global variable buffer (if any)
   if( gv.buf_valid != 0 )
   {
      // just for testing the mechanism
      // you also could place cnt in global_var
      if( gv.receive.command == COUNTER_GET )
      {
         gv_valid( 0 ); // command is accepted by this process
         gv.transmit.pl.u32[0] = cnt;
         gv.transmit.size += 4;
         gv_acknowledge( 1 ); // response of this process is ready
      }
   }


}

// c initialization function 
void counter_init(
   mtiRegionIdT region, // location in the design
   char *parameters, // from vhdl world (not used)
   mtiInterfaceListT *generics, // from vhdl world (not used)
   mtiInterfaceListT *ports // linked list of ports
)
{
   // create a struct to store a link for each vhdl signal
   counter_t *ip = (counter_t *) mti_Malloc( sizeof(counter_t) );

   // map input signals (from vhdl world) to struct
   ip->clk = mti_FindPort( ports, "clk" );
   ip->rst = mti_FindPort( ports, "rst" );
   ip->set = mti_FindPort( ports, "set" );
   ip->inc = mti_FindPort( ports, "inc" );
   ip->dec = mti_FindPort( ports, "dec" );
   ip->load = mti_FindPort( ports, "load" );

   // map "cnt" output signal (to vhdl world) to struct
   ip->cnt = mti_CreateDriver( mti_FindPort( ports, "cnt" ) );

   // create "counter" process with a link to all vhdl signals
   // where the links to the vhdl signals are in the struct
   mtiProcessIdT process_id = mti_CreateProcess( "counter_p", counter, ip );

   // trigger "counter" process when event on vhdl signal clk
   mti_Sensitize( process_id, ip->clk, MTI_EVENT);
}
