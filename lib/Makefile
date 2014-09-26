#############################################################################
#
# Makefile for iot
#
# License: GPL (General Public License)
# Author:  Fergus Leahy <fergus.leahy@gmail.com>
# Date:    25/09/2014 (version 1.0)
#

# The recommended compiler flags for the Raspberry Pi
CFLAGS= -Wall -W -fPIC -g -pthread
RF24=../../../../projects/nrf_lib/RF24/src/
# make all
ifeq ($(platform), pi)
    CFLAGS+=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
endif

DEPS=network.o
OBJECTS=rf24_network.o 
LIBS=librf24.a

all: test

librf24.a: $(RF24)/librf24.a
	cp $(RF24)/librf24.a .

$(RF24)/librf24.a:
	(cd $(RF24); make lib)

# Library parts
network.o: rf24_network.c network.h 
	gcc -c $(CFLAGS) -I$(RF24) rf24_network.c network.h 

pingtest: test.c ${DEPS} ${LIBS}
	gcc ${CFLAGS} pingtest.c ${OBJECTS} ${LIBS} -o test 

clean:
	rm -rf *o ${LIBNAME}
