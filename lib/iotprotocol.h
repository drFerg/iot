/* iot Protocol definition
*
* Hardware and link/transport level independent protocol definition
*
* author Fergus William Leahy
*/ 
#ifndef IOT_PROTOCOL_H
#define IOT_PROTOCOL_H
#include <stdint.h>

//Sensor type
#define TEMP   1
#define HUM    2
#define SWITCH 3
#define LIGHT  4

/* Packet command types */
#define QUERY    1
#define QACK     2
#define CONNECT  3
#define CACK     4
#define RSYN     5
#define RACK     6
#define DISCONNECT 7
#define DACK     8
#define CMD      9
#define CMDACK  10
#define PING    11
#define PACK    12
#define SEQNO   13
#define SEQACK  14
#define RESPONSE 16

#define ASYM_QUERY 17
#define ASYM_RESPONSE 18
#define ASYM_RESP_ACK 19
#define ASYM_KEY_REQ  21
#define ASYM_KEY_RESP 22
#define SYM_HANDOVER  23

#define CMD_LOW QUERY
#define CMD_HIGH KEYACK		/* change this if commands added */

/* =======================*/

#define MAX_PACKET_SIZE    110
#define NO_PAYLOAD          0
#define MAX_DATA_SIZE      100
#define RESPONSE_DATA_SIZE 16

#define NAME_SIZE          16

typedef struct chan_header {

} ChanHeader;

typedef struct payload_header {
   uint8_t src_chan_num;
   uint8_t dst_chan_num;
   uint16_t seqno;   /* sequence number */
   uint8_t cmd;	/* message type */
} PayloadHeader;

typedef struct data_header {
   uint8_t tlen;	/* total length of the data */
} DataHeader;

typedef struct data_payload {		/* template for data payload */
   PayloadHeader hdr;
   DataHeader dhdr;
   uint8_t data[MAX_DATA_SIZE];	/* data is address of MAX_DATA_SIZE bytes */
} DataPayload;

typedef struct packet {
   uint8_t flags;
   ChanHeader ch;
   DataPayload dp;
} Packet;

/********************/
/* Message Payloads */

typedef struct query{
   uint8_t type;
   uint8_t name[NAME_SIZE];
}QueryMsg;

typedef struct query_response{
   uint16_t id;
   uint16_t rate;
   uint8_t type;
   uint8_t name[NAME_SIZE];
}QueryResponseMsg;

typedef struct connect_message{
   uint16_t rate;
}ConnectMsg;

typedef struct cack{
   uint8_t accept;
}ConnectACKMsg;

typedef struct response{
   uint8_t data[MAX_DATA_SIZE];
}ResponseMsg;

typedef struct serial_query_response{
   uint8_t type;
   uint16_t rate;
   uint8_t name[NAME_SIZE];
   uint8_t src;
}SerialQueryResponseMsg;

typedef struct serial_response{
   uint8_t data;
   uint8_t src;
}SerialResponseMsg;

typedef struct serial_connect{
   uint8_t addr;
   uint8_t rate;
}SerialConnect;

typedef struct serial_cack{
   uint8_t accept;
   uint8_t src;
}SerialConnectACKMsg;

#endif /* IOT_PROTOCOL_H */