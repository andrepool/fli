# Copyright 2012, 2013 Andre Pool
# Licensed under the Apache License version 2.0
# you may not use this file except in compliance with this License
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

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

INC = -I. -I../socket -I$$MTI_HOME/include

all: c_environment.so
	@:

%.o: %.c ../socket/sock_config.h ../src/global_var.h
	gcc $(CFLAGS) $(INC) -o $@ -c $<

%.s: %.c ../socket/sock_config.h ../src/global_var.h
	gcc -S $(CFLAGS) $(INC) -o $@ -c $<

%.o: ../socket/%.c ../socket/sock_config.h ../src/global_var.h
	gcc $(CFLAGS) $(INC) -o $@ -c $<

# create one .so file for all c models so we can share the global variable gv
%.so: testbench.o counter.o sqrt_int.o global_var.o housekeeping.o server.o sock_functions.o server_functions.o test_server_functions.o
	gcc $(LDFLAGS) -o $@ $^ -lm -lpthread

clean:
	@sync
	@killall -9 vish 2>/dev/null || true
	@killall -9 vsimk 2>/dev/null || true
	@rm -rf work
	@rm -f *.wlf *.vstf *.vcom transcript
	@$(MAKE) --no-print-directory  MAKEFLAGS= -C ../socket clean
	@rm -f *.o *.so *.s

.PHONY: all clean

.SECONDARY:
