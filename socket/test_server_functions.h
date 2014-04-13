// Copyright 2014 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#ifndef INCLUDED_TEST_SERVER_FUNCTIONS_H
#define INCLUDED_TEST_SERVER_FUNCTIONS_H

#include "global_var.h"

// These functions are only used to test the test server
// They are not used for FLE

/* Function to emulatie interaction with threads that would profide data
  from the simulator
*/
void test_server_provide_data( );

#endif /* INCLUDED_TEST_SERVER_FUNCTIONS_H */
