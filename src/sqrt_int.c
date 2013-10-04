// Copyright 2012, 2013 Andre Pool
// Licensed under the Apache License version 2.0
// you may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#include "mti.h"
#include "global_var.h"
#include <stdio.h>
#include <math.h>

// create one struct that contains all vhdl signals that need to be passed to the function
typedef struct
{
   mtiSignalIdT clk;
   mtiSignalIdT rst;
   mtiSignalIdT input;
   mtiDriverIdT output;
} sqrt_int_t;

static void sqrt_int ( void *param )
{
   _Bool clk, rst;
   mtiInt32T input;
   static mtiInt32T output = 0; // use old value for falling edge clk
   
   // connect input to vhdl
   sqrt_int_t * ip = (sqrt_int_t *) param; // connect function argument to sqrt_int struct
   clk = (_Bool) mti_GetSignalValue ( ip->clk ); // get latest value of vhdl signal
   rst = (_Bool) mti_GetSignalValue ( ip->rst );
   input = mti_GetSignalValue ( ip->input );

   if( clk )
   {
      if( rst )
      {
         output = 0;
      }
      else
      {
         // cast types because sqrt function uses  float, double or long double
         output = (mtiInt32T) ( 0.5 + sqrt( (float) input ) );
      }
      // printf( "INFO    sqrt int of %d returns %d\n", input, output ); fflush(stdout);
   }

   // connect output to vhdl
   mti_ScheduleDriver( ip->output, output, 1, MTI_INERTIAL );

   // process commands from global variable buffer (if any)
   if( gv.buf_valid != 0 )
   {
      if( gv.receive.command == SQRT_INT_GET )
      {
         gv_valid( 0 ); // command is accepted by this process
         gv.transmit.data = output;
         gv_acknowledge( 1 ); // response of this process is ready
      }
   }

}

// c initialization function 
void sqrt_int_init (
   mtiRegionIdT region, // location in the design
   char *param, // last part of the string in the foreign attribute (not used)
   mtiInterfaceListT *generics, // linked list of generics values (not used)
   mtiInterfaceListT *ports // linked list of ports
)
{
   // create one pointer that contains all vhdl signals that need to be passed to the function
   sqrt_int_t *ip;
   ip = (sqrt_int_t *) mti_Malloc( sizeof(sqrt_int_t) );
   ip->clk = mti_FindPort( ports, "clk" ); // finds the "input" signal in the ports interface list
   ip->rst = mti_FindPort( ports, "rst" );
   ip->input = mti_FindPort( ports, "input" );
   ip->output = mti_CreateDriver( mti_FindPort( ports, "output" ) ); // creates driver on "output" signal
   // now pointer "ip" has "links" to all required vhdl signals

   // create handle to vhdl process that calls function "sqrt_int" with function argument "ip"
   mtiProcessIdT process_id;
   process_id = mti_CreateProcess( "sqrt_int_p", sqrt_int, ip );
   // trigger this process when there is an event on vhdl signal "clk"
   mti_Sensitize( process_id, ip->clk, MTI_EVENT);
}
