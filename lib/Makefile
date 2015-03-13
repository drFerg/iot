#############################################################################
#
# Makefile for iot
#
# License: GPL (General Public License)
# Author:  Fergus Leahy <fergus.leahy@gmail.com>
# Date:    25/09/2014 (version 1.0)
#

# The recommended compiler flags for the Raspberry Pi
CFLAGS= -Wall -W -fPIC -g -pthread -DDEBUG
RF24=../../nrf_radio/RF24/src/
# make all
ifeq ($(platform), pi)
    CFLAGS+=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
endif

DEPS= cstate.o ctable.o iot.o
OBJECTS= cstate.o ctable.o iot.o
LIBS=librf24.a

all: test

librf24.a: $(RF24)/librf24.a
	cp $(RF24)/librf24.a .

$(RF24)/librf24.a:
	(cd $(RF24); make lib)

cstate.o: cstate.c cstate.h
ctable.o:cstate.o ctable.c ctable.h
iot.o: cstate.o ctable.o iot.h iot.c

# Library parts
network.o: network_rf24.c network.h 
	gcc -c $(CFLAGS) -I$(RF24) network_rf24.c network.h

dummy.o: network_dummy.c network.h 
	gcc -c $(CFLAGS) -I$(RF24) network_dummy.c network.h 

dummy: test.c ${DEPS} dummy.o
	gcc ${CFLAGS} test.c ${OBJECTS} network_dummy.o -o test

udp: test.c ${DEPS} udp.o
	gcc ${CFLAGS} test.c ${OBJECTS} network_udp.o -o test

udp.o: network_udp.c network.h
	gcc -c $(CFLAGS) -I$(RF24) network_udp.c network.h 

test: test.c ${DEPS} network.o ${LIBS}
	gcc ${CFLAGS} test.c ${OBJECTS} network_rf24.o ${LIBS} -o test 

clean:
	rm -rf *o ${LIBNAME}
