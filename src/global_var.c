#include "global_var.h"

// define global variable which is used to share information between the different threads
global_var_t gv;

// define mutex which prevents problems when different threads are writing to the same field in the global variable
pthread_mutex_t gv_mutex_lock;

void gv_valid( int value )
{
   pthread_mutex_lock( &gv_mutex_lock );
   gv.buf_valid = value;
   pthread_mutex_unlock( &gv_mutex_lock );
}

void gv_acknowledge( int value )
{
   pthread_mutex_lock( &gv_mutex_lock );
   gv.buf_acknowledge = value;
   pthread_mutex_unlock( &gv_mutex_lock );
}
