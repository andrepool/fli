// Copyright 2012, 2013 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#ifndef INCLUDED_GLOBAL_VAR_H
#define INCLUDED_GLOBAL_VAR_H

#include "sock_config.h"

// the global_var_t struct is used to exchange information between the different treads,
// e.g. simulator and server
typedef struct 
{
   double *a;
   int api;
   int sum_pre;
   int sum_post;
   int sum2;
   int veclen;
   sock_buf_t receive;
   sock_buf_t transmit;
   int buf_valid;
   int buf_acknowledge;
   int handshake; // enable or disable handshake between client and testbench
   unsigned int err;
   int sum;
   uint32_t memory[SOCK_BUF_PAYLOAD_SIZE>>2];
} global_var_t;
// Tell other users (threads) of the global variable gv about the type, the actual define of the
// global variable gv is performed in the global_var.c.
extern global_var_t gv;

// Tell other users (threads) of the global mutex lock about the type, the actual define of the
// mutex variable is performed in the global_var.c.
extern pthread_mutex_t gv_mutex_lock;

// Prototype of global variable valid and acknowledge functions, which are made safe by using
// the mutex locking mechanism.
void gv_valid( int value );
void gv_acknowledge( int value );

#endif /* INCLUDED_GLOBAL_VAR_H */


