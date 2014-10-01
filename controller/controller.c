#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include "Timer.h"
#include "ctable.h"
#include "cstate.h"
#include "iotprotocol.h"
#include "iot.h"
#include "network.h"

#if DEBUG
#include "printf.h"
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTFFLUSH(...) printfflush()
#elif SIM
#define PRINTF(...) dbg("DEBUG",__VA_ARGS__)
#define PRINTFFLUSH(...)
#else  
#define PRINTF(...)
#define PRINTFFLUSH(...)
#endif

#define PKT_LEN 32

ChanState home_chan;
	/* Checks the timer for a channel's state, retransmitting when necessary */
void check_timer(ChanState *state) {
    decrement_ticks(state);
    if (ticks_left(state)) return;
    if (attempts_left(state)) {
    	if (in_waiting_state(state))
    		iot_send_on_chan(state, (DataPayload*) &(state->packet));
        else 
        	iot_ping(state); /* PING A LING LONG */
        set_ticks(state, state->ticks * 2); /* Exponential (double) retransmission */
        decrement_attempts(state);
        PRINTF("CLN>> Attempts left %d\n", state->attempts_left);
        PRINTF("CLN>> Retrying packet...\n");
    } else {
        PRINTF("CLN>> CLOSING CHANNEL DUE TO TIMEOUT\n");
        iot_close_graceful(state);
        ctable_remove_channel(state->chan_num);
    }
}

/* Run once every 20ms */
void cleaner(){
	ChanState *state;
	int i = 1;
    for (; i < CHANNEL_NUM; i++) {
    	state = ctable_get_channel_state(i);
        if (state) check_timer(state);
    }
    /*if (home_channel_state.state != STATE_IDLE) {
            check_timer(&home_channel_state);
    }*/
}
  
/*------------------------------------------------- */

void init() {
	PRINTF("\n*********************\n****** BOOTED *******\n*********************\n");
    PRINTFFLUSH();
    ctable_init_table();
    cstate_init_state(&home_chan, 0);
    //set timer for cleaner(TICK_RATE);
}


/*-----------Received packet event, main state event ------------------------------- */
void network_handler(Address *src, uint8_t* payload, uint8_t len) {
	ChanState *state;
    DataPayload *dp = (DataPayload *) payload;
	/* Gets data from the connection */
	uint8_t cmd = dp->hdr.cmd;
	PRINTF("CON>> Received packet from Thing: %s\n", net_ntoa(src));
	PRINTF("CON>> Received a %s command\n", cmdnames[cmd]);
	PRINTF("CON>> Message for channel %d\n", dp->hdr.dst_chan_num);
	PRINTFFLUSH();

	switch(cmd) { /* Drop packets for cmds we don't accept */
        case(QUERY): return;
        case(CONNECT): return;
        case(QACK): iot_qack_handler(&home_chan, dp, src); return;
        case(DACK): return;
	}
    /* Grab state for requested channel */
	state = ctable_get_channel_state(dp->hdr.dst_chan_num);
	if (!state){ /* Attempt to kill connection if no state held */
		PRINTF("Channel %d doesn't exist\n", dp->hdr.dst_chan_num);
		state = &home_chan;
		state->remote_chan_num = dp->hdr.src_chan_num;
		net_addrcpy(state->remote_addr, src);
		state->seqno = dp->hdr.seqno;
		iot_close_graceful(state);
		return;
	} else if (!iot_valid_seqno(state, dp)) {
		PRINTF("Old packet\n");
		return;
	}
	switch(cmd) {
		case(CACK): iot_controller_cack_handler(state, dp); break;
		case(RESPONSE): iot_response_handler(state, dp); break;
		case(RSYN): iot_response_handler(state, dp); iot_send_rack(state); break;
		// case(CMDACK):   	command_ack_handler(state,dp);break;
		case(PING): iot_ping_handler(state, dp); break;
		case(PACK): iot_pack_handler(state, dp); break;
		case(DISCONNECT): iot_disconnect_handler(state, dp); 
						  ctable_remove_channel(state->chan_num); break;
		default: PRINTF("Unknown CMD type\n");
	}
}

// void local_handler(uint8_t *msg, void* payload, uint8_t len){
// 	DataPayload *dp = (DataPayload *)payload;
// 	void * data = &(dp->data);
// 	uint8_t cmd = dp->hdr.cmd;
// 	iot_report_received();
	
// 	PRINTF("SERIAL> Serial command received.\n");
// 	PRINTF("SERIAL> Packet length: %d\n", dp->dhdr.tlen);
// 	PRINTF("SERIAL> Message for channel %d\n", dp->hdr.dst_chan_num);
// 	PRINTF("SERIAL> Command code: %d\n", dp->hdr.cmd);
// 	PRINTFFLUSH();

// 	switch (cmd) {
// 		case(QUERY): iot_query(&home_chan, 1/*((QueryMsg*)dp)->type*/);break;
// 		case(CONNECT): iot_connect(ctable_new_channel(), 
// 									((SerialConnect*)data)->addr, 
// 									((SerialConnect*)data)->rate);break;
// 	}
// 	iot_report_received();
// }

// void sendDone(message_t *msg, error_t error){
// 	if (error == SUCCESS) iot_report_sent();
//     else iot_report_problem();
//     serialSendBusy = FALSE;
// }

int main(){
    int len = 0;
    uint8_t pkt[PKT_LEN];
    Address *src = net_addralloc();
    int status = net_init();
    if (status == 0){
        printf("Catastrophic error, exiting.\n");
        return -1;
    }
    while (1){
        len = net_recvfrom(pkt, PKT_LEN, src, SYNC);
        network_handler(src, pkt, len);
    }
    return 0;
}
