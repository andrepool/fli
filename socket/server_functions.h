// Copyright 2012 - 2015 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#ifndef INCLUDED_SERVER_FUNCTIONS_H
#define INCLUDED_SERVER_FUNCTIONS_H

#include "global_var.h"

/* Function for the server to exchange data with the client
 - server is connected with client
 - receive data from client
 - handle command from client
 - create response
 - send response to client
*/
void server_exchange_with_client( int *client_sock_file_desc, int *shutdown );

#endif /* INCLUDED_SERVER_FUNCTIONS_H */
