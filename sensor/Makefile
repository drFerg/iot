#############################################################################
#
# Makefile for sensor
#
# License: GPL (General Public License)
# Author:  Fergus Leahy <fergus.leahy@gmail.com>
# Date:    25/09/2014 (version 1.0)
#

# The recommended compiler flags for the Raspberry Pi
CFLAGS= -Wall -W -fPIC -g
IOT=../lib
# make all
ifeq ($(platform), pi)
    CFLAGS+=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
endif

DEPS=
OBJECTS=cstate.o ctable.o iot.o
OBJ=${addprefix ../lib/, ${OBJECTS}}
ADTSOBJ=${ADTS}/tsuqueue.o

LIBS=-lrf24 ${ADTS}/libADTs.a -pthread
ADTS=../adts


all: sensor

# Library parts
objs: 
	(cd ${IOT}; make)

dummy_objs: 
	(cd ${IOT}; make dummy)

udp_objs: 
	(cd ${IOT}; make udp)

libADTs.a:
	(cd ${ADTS}; make)

sensor: sensor.c objs
	gcc ${CFLAGS} -I${IOT} -I${ADTS} -L${IOT} sensor.c ../lib/network_rf24.o ${OBJ} ${LIBS} -o sensor 

dummy_sensor: sensor.c dummy_objs ${ADTS}/tsuqueue.o ${ADTS}/tsuqueue.h
	gcc ${CFLAGS} -I${IOT} -I${ADTS} -L${IOT} sensor.c ../lib/network_dummy.o ${OBJ} ${LIBS} -o sensor

udp_sensor: sensor.c udp_objs ${ADTS}/tsuqueue.o ${ADTS}/tsuqueue.h
	gcc ${CFLAGS} -I${IOT} -I${ADTS} -L${IOT} sensor.c ../lib/network_udp.o ${OBJ} ${LIBS} -o sensor

clean:
	rm -rf *o ${LIBNAME}
