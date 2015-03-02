#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "iot.h"

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#elif SIM
#define PRINTF(...) dbg("DEBUG",__VA_ARGS__)
#define PRINTFFLUSH(...)
#else  
#define PRINTF(...)
#define PRINTFFLUSH(...)
#endif

#define SEQNO_START 0
#define SEQNO_LIMIT 255
#ifndef DEVICE_NAME
#define DEVICE_NAME "WHOAMI"
#endif
#ifndef SENSOR_TYPE
#define SENSOR_TYPE 0
#endif
#ifndef DATA_RATE
#define DATA_RATE 10
#endif

uint8_t pkt[20];
extern const char* cmdnames[24] = {"DUMMY0", "QUERY", "QACK","CONNECT", "CACK", 
                            "RSYN", "RACK", "DISCONNECT", "DACK",
                            "COMMAND", "COMMANDACK", "PING", "PACK", "SEQNO",
                            "SEQACK", "DUMMY1", "RESPONSE", "ASYM_QUERY",
                            "ASYM_RESP", "ASYM_RESP_ACK", "DUMMY2", 
                            "ASYM_KEY_REQ", "ASYM_KEY_RESP", "KEY_HANDOVER"};

void increment_seq_no(ChanState *state, DataPayload *dp){
	if (state->seqno >= SEQNO_LIMIT){
		state->seqno = SEQNO_START;
	} else {
		state->seqno++;
	}
	dp->hdr.seqno = state->seqno;
}

void send(Address *addr, DataPayload *dp){
	uint8_t len = sizeof(PayloadHeader) + sizeof(DataHeader) + dp->dhdr.tlen;
	if (net_sendto(addr, dp, len) > 0) {
		PRINTF("RADIO>> Sent a %s packet to Thing %s\n", cmdnames[dp->hdr.cmd], net_ntoa(addr));		
		PRINTF("RADIO>> iot Payload Length: %d\n", dp->dhdr.tlen);
	}
	else {
		PRINTF("ID: Radio Msg could not be sent, channel busy\n");
	}
	
}

void dp_complete(DataPayload *dp, uint8_t src, uint8_t dst, 
             uint8_t cmd, uint8_t len){
	dp->hdr.src_chan_num = src; 
	dp->hdr.dst_chan_num = dst; 
	dp->hdr.cmd = cmd; 
	dp->dhdr.tlen = len;
}

int iot_valid_seqno(ChanState *state, DataPayload *dp){
	if (state->seqno > dp->hdr.seqno){ // Old packet or sender confused
		return 0;
	} else {
		state->seqno = dp->hdr.seqno;
		if (state->seqno >= SEQNO_LIMIT){
			state->seqno = SEQNO_START;
		}
		return 1;
	}
}

void iot_send_on_chan(ChanState *state, DataPayload *dp){
	increment_seq_no(state, dp);
	send(state->remote_addr, dp);
}

void iot_knot_broadcast(ChanState *state, DataPayload *dp){
	increment_seq_no(state, dp);
	send(net_broadaddr(), dp);
}


/* Higher level calls */

/***** QUERY CALLS AND HANDLERS ******/
void iot_query(ChanState* state, uint8_t type){
	DataPayload *new_dp = (DataPayload *)&(state->packet); 
	QueryMsg *q;
    clean_packet(new_dp);
    dp_complete(new_dp, HOME_CHANNEL, HOME_CHANNEL, 
             QUERY, sizeof(QueryMsg));
    q = (QueryMsg *) new_dp->data;
    q->type = type;
    strcpy((char*)q->name, DEVICE_NAME);
    iot_knot_broadcast(state, new_dp);
    set_ticks(state, TICKS);
    set_state(state, STATE_QUERY);
    // Set timer to exit Query state after 5 secs~
}

void iot_query_handler(ChanState *state, DataPayload *dp, Address *src){
	DataPayload *new_dp;
	QueryResponseMsg *qr;
	QueryMsg *q = (QueryMsg*)(dp->data);
	if (q->type != SENSOR_TYPE) {
		PRINTF("Query doesn't match type\n");
		return;
	}
	PRINTF("Query matches type\n");
	state->remote_addr = net_addrcpy(src);
	new_dp = (DataPayload *)&(state->packet);
	qr = (QueryResponseMsg*)&(new_dp->data);
	clean_packet(new_dp);
	strcpy((char*)qr->name, DEVICE_NAME); /* copy name */
	qr->type = SENSOR_TYPE;
	qr->rate = DATA_RATE;
	dp_complete(new_dp, state->chan_num, dp->hdr.src_chan_num, 
				QACK, sizeof(QueryResponseMsg));
	iot_send_on_chan(state, new_dp);
	net_addrfree(state->remote_addr);
}

void iot_qack_handler(ChanState *state, DataPayload *dp, Address *src) {
	SerialQueryResponseMsg *qr;
	if (state->state != STATE_QUERY) {
		PRINTF("iot>> Not in Query state\n");
		return;
	}
    state->remote_addr = net_addrcpy(src);
}

/*********** CONNECT CALLS AND HANDLERS ********/

void iot_connect(ChanState *state, Address *addr, int rate){
	ConnectMsg *cm;
	DataPayload *new_dp;
	state->remote_addr = addr;
	state->rate = rate;
	new_dp = (DataPayload *)&(state->packet);
	clean_packet(new_dp);
	dp_complete(new_dp, state->chan_num, HOME_CHANNEL, 
             CONNECT, sizeof(ConnectMsg));
	cm = (ConnectMsg *)(new_dp->data);
	cm->rate = rate;
    iot_send_on_chan(state, new_dp);
    set_ticks(state, TICKS);
    set_attempts(state, ATTEMPTS);
    set_state(state, STATE_CONNECT);
   	PRINTF("iot>> Sent connect request from chan %d\n", state->chan_num);
    
	
}

void iot_connect_handler(ChanState *state, DataPayload *dp, Address *src){
	ConnectMsg *cm;
	DataPayload *new_dp;
	ConnectACKMsg *ck;
	state->remote_addr = net_addrcpy(src);
	printf("%s\n", net_ntoa(src));
	printf("%s\n", net_ntoa(state->remote_addr));
	cm = (ConnectMsg*)dp->data;
	/* Request src must be saved to message back */
	state->remote_chan_num = dp->hdr.src_chan_num;
	if (cm->rate > DATA_RATE) state->rate = cm->rate;
	else state->rate = DATA_RATE;
	new_dp = (DataPayload *)&(state->packet);
	ck = (ConnectACKMsg *)&(new_dp->data);
	clean_packet(new_dp);
	dp_complete(new_dp, state->chan_num, state->remote_chan_num, 
				CACK, sizeof(ConnectACKMsg));
	ck->accept = 1;
	iot_send_on_chan(state, new_dp);
	set_ticks(state, TICKS);
	set_attempts(state, ATTEMPTS);
	set_state(state, STATE_CONNECT);
	PRINTF("iot>> %s wants to connect from channel %d\n", net_ntoa(src), state->remote_chan_num);
	
	PRINTF("iot>> Replying on channel %d\n", state->chan_num);
	PRINTF("iot>> The rate is set to: %d\n", state->rate);
	
}

uint8_t iot_controller_cack_handler(ChanState *state, DataPayload *dp){
	ConnectACKMsg *ck = (ConnectACKMsg*)(dp->data);
	DataPayload *new_dp;
	SerialConnectACKMsg *sck;
	if (state->state != STATE_CONNECT){
		PRINTF("iot>> Not in Connecting state\n");
		return -1;
	}
	if (ck->accept == 0){
		PRINTF("iot>> SCREAM! THEY DIDN'T EXCEPT!!\n");
		return 0;
	}
	PRINTF("iot>> %s accepts connection request on channel %d\n", 
		net_ntoa(state->remote_addr),
		dp->hdr.src_chan_num);
	state->remote_chan_num = dp->hdr.src_chan_num;
	new_dp = (DataPayload *)&(state->packet);
	clean_packet(new_dp);
	dp_complete(new_dp, state->chan_num, state->remote_chan_num, 
             CACK, NO_PAYLOAD);
	iot_send_on_chan(state,new_dp);
	set_ticks(state, ticks_till_ping(state->rate));
	set_attempts(state, ATTEMPTS);
	set_state(state, STATE_CONNECTED);
	//Set up ping timeouts for liveness if no message received or
	// connected to actuator
	//sck = (SerialConnectACKMsg *) ck;
	//sck->src = state->remote_addr;
	//send_on_serial(dp);
	return 1;
}

uint8_t iot_sensor_cack_handler(ChanState *state, DataPayload *dp){
	if (state->state != STATE_CONNECT){
		PRINTF("iot>> (C%d) Not in Connecting state\n", state->chan_num);
		return 0;
	}
	PRINTF("iot>> (C%d) Received final CACK from Controller\n", state->chan_num);
	PRINTF("iot>> (C%d) TX rate: %d\n", state->chan_num, state->rate);
	PRINTF("iot>> (C%d) CONNECTION FULLY ESTABLISHED<<\n", state->chan_num);
	set_ticks(state, ticks_till_ping(state->rate));
	set_state(state, STATE_CONNECTED);
	return 1;
}

/**** RESPONSE CALLS AND HANDLERS ***/
void iot_send_value(ChanState *state, uint8_t *data, uint8_t len){
    DataPayload *new_dp = (DataPayload *)&(state->packet);
	ResponseMsg *rmsg = (ResponseMsg*)&(new_dp->data);
	// Send a Response SYN or Response
	if (state->state == STATE_CONNECTED){
		clean_packet(new_dp);
		new_dp->hdr.cmd = RESPONSE;
    	state->ticks_till_ping--;
    } else if(state->state == STATE_RSYN){
    	clean_packet(new_dp);
	    new_dp->hdr.cmd = RSYN; // Send to ensure controller is still out there
	    state->ticks_till_ping = RSYN_RATE;
	    set_state(state, STATE_RACK_WAIT);
    } else if (state->state == STATE_RACK_WAIT){
    	return; /* Waiting for response, no more sensor sends */
    }
    memcpy(&(rmsg->data), data, len);
    new_dp->hdr.src_chan_num = state->chan_num;
	new_dp->hdr.dst_chan_num = state->remote_chan_num;
    new_dp->dhdr.tlen = sizeof(ResponseMsg);
    PRINTF("Sending data\n");
    iot_send_on_chan(state, new_dp);
}

void iot_response_handler(ChanState *state, DataPayload *dp){
	ResponseMsg *rmsg;
	SerialResponseMsg *srmsg;
	uint8_t temp;
	if (state->state != STATE_CONNECTED && state->state != STATE_PING){
		PRINTF("iot>> Not connected to device!\n");
		return;
	}
	set_ticks(state, ticks_till_ping(state->rate)); /* RESET PING TIMER */
	set_attempts(state, ATTEMPTS);
	rmsg = (ResponseMsg *)dp->data;
	memcpy(&temp, &(rmsg->data), 1);
	PRINTF("iot>> Data rvd: %d\n", temp);
	PRINTF("Temp: %d\n", temp);
	//srmsg = (SerialResponseMsg *)dp->data;
	//srmsg->src = state->remote_addr;
	//send_on_serial(dp);
}

void iot_send_rack(ChanState *state){
	DataPayload *new_dp = (DataPayload *)&(state->packet);
	clean_packet(new_dp);
	dp_complete(new_dp, state->chan_num, state->remote_chan_num, 
             RACK, NO_PAYLOAD);
	iot_send_on_chan(state, new_dp);
}

void iot_rack_handler(ChanState *state, DataPayload *dp){
	if (state->state != STATE_RACK_WAIT){
		PRINTF("iot>> Didn't ask for a RACK!\n");
		return;
	}
	set_state(state, STATE_CONNECTED);
	set_ticks(state, ticks_till_ping(state->rate));
	set_attempts(state, ATTEMPTS);
}

/*** PING CALLS AND HANDLERS ***/
void iot_ping(ChanState *state){
	DataPayload *new_dp = (DataPayload *)&(state->packet);
	clean_packet(new_dp);
	dp_complete(new_dp, state->chan_num, state->remote_chan_num, 
	           PING, NO_PAYLOAD);
	iot_send_on_chan(state, new_dp);
	set_state(state, STATE_PING);
}

void iot_ping_handler(ChanState *state, DataPayload *dp){
	DataPayload *new_dp;
	if (state->state != STATE_CONNECTED) {
		PRINTF("iot>> Not in Connected state\n");
		return;
	}
	new_dp = (DataPayload *)&(state->packet);
	clean_packet(new_dp);
	dp_complete(new_dp, state->chan_num, state->remote_chan_num, 
	           PACK, NO_PAYLOAD);
	iot_send_on_chan(state,new_dp);
}

void iot_pack_handler(ChanState *state, DataPayload *dp){
	if (state->state != STATE_PING) {
		PRINTF("iot>> Not in PING state\n");
		return;
	}
	set_state(state, STATE_CONNECTED);
	set_ticks(state, ticks_till_ping(state->rate));
	set_attempts(state, ATTEMPTS);
}

/*** DISCONNECT CALLS AND HANDLERS ***/
void iot_close_graceful(ChanState *state){
	DataPayload *new_dp = (DataPayload *)&(state->packet);
	clean_packet(new_dp);
	dp_complete(new_dp, state->chan_num, state->remote_chan_num, 
	           DISCONNECT, NO_PAYLOAD);
	iot_send_on_chan(state,new_dp);
	set_state(state, STATE_DCONNECTED);
}
void iot_disconnect_handler(ChanState *state, DataPayload *dp){
	DataPayload *new_dp = (DataPayload *)&(state->packet);
	clean_packet(new_dp);
	dp_complete(new_dp, dp->hdr.src_chan_num, state->remote_chan_num, 
               DACK, NO_PAYLOAD);
	iot_send_on_chan(state, new_dp);
}