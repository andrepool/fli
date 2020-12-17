// Copyright 2012 - 2015 Andre Pool
// Licensed under the Apache License version 2.0
// You may not use this file except in compliance with this License
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

#include "sock_functions.h"

#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <netdb.h> 

#ifdef USE_AF_INIT
int sock_server_setup_and_listen( )
{
   int server_sock_file_desc, bind_cnt = 0, bind_val = 0;
   struct sockaddr_in server_addr;

   // create the correct "server socket" struct
   bzero( (char *) &server_addr, sizeof(server_addr) );
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = INADDR_ANY;
   server_addr.sin_port = htons(TCP_IP_PORT);

   if( ( server_sock_file_desc = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
   {
      printf("ERROR   errno message '%s', server cannot create socket!\n", strerror(errno) );
      exit( EXIT_FAILURE );
   }

   bind_val = bind( server_sock_file_desc, (struct sockaddr *)&server_addr, sizeof(server_addr) );
   while( bind_val < 0 )
   {
      if( bind_cnt % 10 == 0 ) {
         printf("WARNING errno message '%s', server cannot bind to socket, try %d\n", strerror(errno), bind_cnt );
      }
      bind_val = bind( server_sock_file_desc, (struct sockaddr *)&server_addr, sizeof(server_addr) );
      if( bind_cnt > 100 ) {
         printf("ERROR   give up\n" );
         exit( EXIT_FAILURE );
      }
      bind_cnt++;
   }

   if( listen( server_sock_file_desc, 5 ) < 0 )
   {
      printf("ERROR   errno message '%s', server cannot listen on socket\n", strerror(errno));
      exit( EXIT_FAILURE );
   }
   return server_sock_file_desc;
}

#else

int sock_server_setup_and_listen( )
{
   int server_sock_file_desc;
   struct sockaddr_un server_addr;
   socklen_t server_addr_length;

   // create the correct "server socket" struct
   bzero( (char *) &server_addr, sizeof(server_addr) );
   server_addr.sun_family = AF_UNIX;
   strcpy(server_addr.sun_path, SOCK_FILE_HANDLE );
   server_addr_length = strlen(server_addr.sun_path) + sizeof(server_addr.sun_family);

   if( ( server_sock_file_desc = socket( AF_UNIX, SOCK_STREAM, 0 ) ) < 0 )
   {
      printf("ERROR   errno message '%s', server cannot create socket!\n", strerror(errno) );
      exit( EXIT_FAILURE );
   }

   // cleanup old file handle if exist   
   if( ( access( SOCK_FILE_HANDLE, F_OK ) != -1 ) && ( unlink(SOCK_FILE_HANDLE) < 0 ) )
   {
      printf( "ERROR   errno message '%s', cannot unlink existing file handle\n", strerror( errno ) );
      exit( EXIT_FAILURE );
   }

   if( bind( server_sock_file_desc, (struct sockaddr *)&server_addr, server_addr_length ) < 0 )
   {
      printf("ERROR   errno message '%s', server cannot bind to socket\n", strerror(errno) );
      exit( EXIT_FAILURE );
   }

   if( listen( server_sock_file_desc, 5 ) < 0 )
   {
      printf("ERROR   errno message '%s', server cannot listen on socket\n", strerror(errno));
      exit( EXIT_FAILURE );
   }
   return server_sock_file_desc;
}
#endif

int sock_client_setup( )
{
   int client_sock_file_desc;
#ifdef USE_AF_INIT
   if( ( client_sock_file_desc = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
#else
   if( ( client_sock_file_desc = socket( AF_UNIX, SOCK_STREAM, 0 ) ) < 0 )
#endif
   {
      printf( "ERROR   errno message '%s', client cannot create socket\n", strerror(errno) );
      exit( EXIT_FAILURE );
   }
   return client_sock_file_desc;
}

#ifdef USE_AF_INIT
void sock_client_connect_to_server( int client_sock_file_desc )
{
   struct sockaddr_in server_addr;
   struct hostent *server;
   
   server = gethostbyname(TCP_IP_ADDRESS);
   if( server == NULL ) {
      printf( "ERROR no such host\n" );
      exit(0);
   }
   bzero( (char *) &server_addr, sizeof(server_addr) ); // initialize
   server_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, server->h_length);
   server_addr.sin_port = htons(TCP_IP_PORT);

   // connect client socket to server socket
   if( connect( client_sock_file_desc, (struct sockaddr *) &server_addr, sizeof(server_addr) ) < 0 )
   {
      printf( "ERROR   errno message '%s', client cannot connect to server\n", strerror(errno) );
      exit( EXIT_FAILURE );
   }
}
#else
void sock_client_connect_to_server( int client_sock_file_desc )
{
   struct sockaddr_un server_addr;
   bzero( (char *) &server_addr, sizeof(server_addr) ); // initialize
   server_addr.sun_family = AF_UNIX;
   strcpy(server_addr.sun_path, SOCK_FILE_HANDLE );
   socklen_t server_addrlen=strlen(server_addr.sun_path) + sizeof(server_addr.sun_family);

   while( access( SOCK_FILE_HANDLE, F_OK ) == -1 )
   {
       printf( "WARNING wait for server socket\n" );
       sleep( 1 );
   }

   // connect client socket to server socket
   if( connect( client_sock_file_desc, (struct sockaddr *) &server_addr, server_addrlen ) < 0 )
   {
      printf( "ERROR   errno message '%s', client cannot connect to server\n", strerror(errno) );
      exit( EXIT_FAILURE );
   }
}
#endif

#ifdef USE_AF_INIT
int sock_server_wait_for_client(  int server_sock_file_desc )
{
   int client_sock_file_desc;

   struct sockaddr_in client_addr;

   socklen_t client_addr_length = sizeof(client_addr);
   if( ( client_sock_file_desc = accept( server_sock_file_desc, (struct sockaddr *)&client_addr, &client_addr_length ) ) < 0 )
   {
      printf( "ERROR   errno message '%s', socket is not accepting\n", strerror(errno) );
      exit( EXIT_FAILURE );
   }
   
   return client_sock_file_desc;
}

#else

int sock_server_wait_for_client(  int server_sock_file_desc )
{
   int client_sock_file_desc;

   struct sockaddr_un client_addr;

   socklen_t client_addr_length = sizeof(client_addr);
   if( ( client_sock_file_desc = accept( server_sock_file_desc, (struct sockaddr *)&client_addr, &client_addr_length ) ) < 0 )
   {
      printf( "ERROR   errno message '%s', socket is not accepting\n", strerror(errno) );
      exit( EXIT_FAILURE );
   }
   
   return client_sock_file_desc;
}
#endif

ssize_t sock_receive_data( int *sock_file_desc, sock_buf_t *buf )
{
   ssize_t bytes_received, bytes_total = 0;
   char *buf_char = (char *) buf; // cast to buf char pointer which we can easy increment by the amount of received bytes
   do
   {
      if( ( bytes_received = recv( *sock_file_desc, buf_char, sizeof(sock_buf_t), 0 ) ) < 0 )
      {
         printf( "ERROR   errno message '%s' when receiving data\n", strerror(errno) );
      }
      buf_char += bytes_received; // move buf char pointer to the place where we ended with this read, which is
      // is used for the following read (if required)
      bytes_total += bytes_received;
   }
   // in case of an unexpected disconnect the function returns zero
   while( ( bytes_received > 0 ) && ( bytes_total < (ssize_t) buf->size ) );
   
   // returns -1 in case of a connection error and 0 in case of an unexpected disconnect
   return bytes_total;
}

int sock_transmit_data( int *sock_file_desc, sock_buf_t *buf )
{
   int bytes_transmitted, bytes_total = 0;
   do
   {
      if( ( bytes_transmitted = send( *sock_file_desc, buf, buf->size, MSG_NOSIGNAL ) ) < 0 )
      {
         printf( "ERROR   errno message '%s' when tranmitting data\n", strerror(errno) );
      }
      bytes_total += bytes_transmitted;
   }
   // in case of an unexpected disconnect the function returns zero
   while( ( bytes_transmitted > 0 ) && ( bytes_total < buf->size ) );

   // returns -1 in case of a connenction error and 0 in case of an unexpected disconnect
   return bytes_total;
}

/* the print  %'d is not supported by ansi c, the following function creates
a string with the thousand comma separator from a number */
void thousands_comma( char *strptr, size_t order, uint64_t number )
{
   uint64_t divider = (uint64_t) pow ( 1000, order );
   int first_found = 0;
   bzero( strptr, strlen(strptr) );
   while( divider != 0 )
   {
      uint64_t thousends = number/divider;
      if( first_found == 0 )
      {
         if( thousends == 0 )
         {
            sprintf( strptr, "%s    ", strptr );
         }
         else
         {
            first_found = 1;
            sprintf( strptr, "%s %3u", strptr, (uint32_t) thousends );
         }
      }
      else
      {
         sprintf( strptr, "%s,%03u", strptr, (uint32_t) thousends );
      }
      number = number - thousends*divider;
      divider = divider/1000;
   }
}       
