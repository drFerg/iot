#############################################################################
#
# Makefile for controller
#
# License: GPL (General Public License)
# Author:  Fergus Leahy <fergus.leahy@gmail.com>
# Date:    25/09/2014 (version 1.0)
#

# The recommended compiler flags for the Raspberry Pi
CFLAGS= -Wall -W -fPIC -g -pthread
IOT=../lib
# make all
ifeq ($(platform), pi)
    CFLAGS+=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
endif

DEPS=
OBJECTS=rf24_network.o cstate.o ctable.o iot.o
OBJ=${addprefix ../lib/, ${OBJECTS}}
LIBS=-lrf24

all: controller

# Library parts
objs: 
	(cd ${IOT}; make)

controller: controller.c objs
	gcc ${CFLAGS} -I${IOT} -L${IOT} controller.c ${OBJ} ${LIBS} -o controller 

clean:
	rm -rf *o ${LIBNAME}
