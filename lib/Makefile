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
network.o: rf24_network.c network.h 
	gcc -c $(CFLAGS) -I$(RF24) rf24_network.c network.h

dummy.o: dummy_network.c network.h 
	gcc -c $(CFLAGS) -I$(RF24) dummy_network.c network.h 

dummy: test.c ${DEPS} dummy.o ${LIBS}
	gcc ${CFLAGS} test.c ${OBJECTS} dummy_network.o ${LIBS} -o test 

test: test.c ${DEPS} network.o ${LIBS}
	gcc ${CFLAGS} test.c ${OBJECTS} rf24_network.o ${LIBS} -o test 

clean:
	rm -rf *o ${LIBNAME}
