#ifndef IOT_H
#define IOT_H
#include <stdint.h>
#include "iotprotocol.h"
#include "cstate.h"
#include "network.h"

#define HOME_CHANNEL 0
typedef uint8_t bool;
#define FALSE 0
#define TRUE 1
/*
* the following definitions control the exponential backoff retry
* mechanism used in the protocol - these may also be changed using
* -D<symbol>=value in CFLAGS in the Makefile
*/
#define TICK_RATE 100 /* tick rate in ms */
#define ATTEMPTS 5 /* number of attempts before removing channel */
#define TICKS 5 /* initial number of 20ms ticks before first retry
                 * number of ticks is doubled for each successive retry */
#define ticks_till_ping(send_rate)((3000 * send_rate)/TICK_RATE) /* rate\s * 3000ms / TICK_RATE= 3xrate = will ping after */
                               /* 3 packets have not been received. */
#define RSYN_RATE 15 /* Rate to send out a RSYN message */

/* Memsets a Datapayload */
#define clean_packet(dp) (memset(dp, 0, sizeof(DataPayload)))

/* Checks the sequence number and returns 1 if in sequence, 0 otherwise */
int iot_valid_seqno(ChanState *state, DataPayload *dp);

/* Sends DataPayload on a KNoT channel specified in the state */
void iot_send_on_chan(ChanState *state, DataPayload *dp);

/* Sends a DataPayload as a broadcast transmission */
void iot_knot_broadcast(ChanState *state, DataPayload *dp);

void iot_query(ChanState* state, uint8_t type);

void iot_query_handler(ChanState *state, DataPayload *dp, Address *src);

void iot_qack_handler(ChanState *state, DataPayload *dp, Address *src);

void iot_connect(ChanState *new_state, Address *addr, int rate);

void iot_connect_handler(ChanState *state, DataPayload *dp, Address *src);

uint8_t iot_controller_cack_handler(ChanState *state, DataPayload *dp);

uint8_t iot_sensor_cack_handler(ChanState *state, DataPayload *dp);

void iot_send_value(ChanState *state, uint8_t *data, uint8_t len);

ResponseMsg *iot_response_handler(ChanState *state, DataPayload *dp);

void iot_send_rack(ChanState *state);
void iot_rack_handler(ChanState *state, DataPayload *dp);

/* Sends a ping packet to the channel in state */
void iot_ping(ChanState *state);

/* Handles the reception of a PING packet, replies with a PACK */
void iot_ping_handler(ChanState *state, DataPayload *dp);

/* Handles the reception of a PACK packet */
void iot_pack_handler(ChanState *state, DataPayload *dp);

/* Closes the channel specified and sends out a DISCONNECT packet */
void iot_close_graceful(ChanState *state);

/* Handles the reception of a DISCONNECT packet */
void iot_disconnect_handler(ChanState *state, DataPayload *dp);

void iot_receive(Address *src, void *payload, uint8_t len);

#endif /* IOT_H */