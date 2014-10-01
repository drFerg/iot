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
#define SYM_KEY_SIZE       10
#define ASYM_SIZE          84
#define NAME_SIZE          16
#define MAC_SIZE            4
#define E_NONCE_SIZE       45 /* 4(nonce) + 20(KEY_SIZE) + 1 + 20(HMAC) */ 
#define E_KEY_SIZE         55 /* 4(nonce) + 10(SYM_KEY_SIZE)+ 20(KEY_SIZE) + 1 + 20(HMAC) */
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

/*********************/
/* Asymmetric Packet */
typedef struct pubKey {
   uint16_t x[10];
   uint16_t y[10];
} PubKey; /* 40bytes */

typedef struct sig {
   uint16_t r[11];
   uint16_t s[11];
} Signature; /* 42bytes */

typedef struct pkc {
   PubKey pubKey;
   Signature sig;
} PKC; /* 80bytes */

typedef struct asym_query_payload {
   PKC pkc;
   uint8_t flags; /* handshake/cipherSpec? */
} AsymQueryPayload; /* 81bytes */

typedef struct asym_resp_ack_payload {
   uint8_t flags;
} AsymRespACKPayload; /* 81bytes */

typedef struct asym_request_payload {
   uint8_t e_nonce[E_NONCE_SIZE];
   Signature sig;
} AsymKeyRequestPayload; /* 85bytes */

typedef struct asym_key_payload {
   uint32_t nonce;
   uint8_t sKey[10];
} AsymKeyPayload;

typedef struct asym_key_tx_payload {
   uint8_t e_payload[E_KEY_SIZE];
   Signature sig;
} AsymKeyRespPayload;

/********************/
/* Symmetric Packet */
typedef struct sec_header {
   uint8_t tag[MAC_SIZE];
} SSecHeader;

typedef struct symmetric_secure_data_payload {
   uint8_t flags;
   /* 1 byte Pad */   
   SSecHeader sh;
   ChanHeader ch;
   DataPayload dp;
} SSecPacket;

/****************/
/* Plain Packet */
typedef struct plain_data_payload {
   ChanHeader ch;
   DataPayload dp;
} PDataPayload;

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