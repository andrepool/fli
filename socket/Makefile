# Copyright 2012, 2013 Andre Pool
# Licensed under the Apache License version 2.0
# You may not use this file except in compliance with this License
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

# to set c #define from command line do: make CFLAGS=-DMY_DEFINE

MAKEFLAGS = -j8

CFLAGS = -g -O2 -Wall -ansi -fms-extensions -std=c99
CFLAGS += -pedantic
LDFLAGS = -shared -Wl,-Bsymbolic -Wl,-export-dynamic -std=c99
ifeq ($(shell uname -m), x86_64)
   CFLAGS += -m64 -fPIC
   LDFLAGS += -m64
else
   CFLAGS += -m32
   LDFLAGS += -m32
endif
# use next flag when FLI application returns real or time values
CFLAGS += -freg-struct-return

INC = -I. -I../src -I$$MTI_HOME/include

all: client

%.o: %.c sock_config.h ../src/global_var.h
	gcc $(CFLAGS) $(INC) -o $@ -c $<

%.o: ../src/%.c sock_config.h ../src/global_var.h
	gcc $(CFLAGS) $(INC) -o $@ -c $<

# control_application.o : CFLAGS := $(filter-out -pedantic,$(CFLAGS))

control_application: control_application.o sock_functions.o
	gcc $(CFLAGS) -o $@ $^ -lm

client: control_application
	@./$< -p "####    first message from $< ####"
	@./$< -t # some basic testing
	@./$< -m "pwd" # test modelsim tcl command
	@./$< -v -l 10 # show hierarchy and connect 10 times
	@echo "INFO    done with testing, send one last message to server"
	@./$< -p "####    last  message from $< ####"

inf: control_application
	@./$< -p "####    first message from $< ####" -t -w -m "pwd"
	@./$< -l -1
	@echo "INFO    done with testing, send one last message to server"
	@./$< -p "####    last  message from $< ####"

quit: control_application
	@echo "INFO    control application request to quit simulator"
	@./$< -p "####    now shutdown ####" -q


clientq: client sleep quit

sleep: 
	@sleep 2

test_server: test_server.o global_var.o sock_functions.o server_functions.o
	gcc $(CFLAGS) -o $@ $^ -lm

test: test_server
	@./$<

clean:
	@killall sock_client sock_server 2>/dev/null || true
	@sync
	@rm -f *.o
	@rm -f test_server
	@rm -f control_application
	@rm -f sock_file_handle

.PHONY: all client clientq quit sleep test clean

.SECONDARY:
