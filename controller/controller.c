#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

//#include "Timer.h"
#include "tsuqueue.h"
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
#define NETWORK_EVENT 1
#define UI_EVENT      2
#define HWDB_EVENT    3

typedef struct event {
    int type;
    int len;
    uint8_t pkt[PKT_LEN];
    Address *src;
    int command;
    char arg[20];
} Event;


int len = 0;
uint8_t pkt[PKT_LEN];
Address *src;
int com;
char arg[20];
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

void ui_handler(int command, char *arg){
    switch(command){
        case(1): iot_query(&home_chan, (int)arg[0]); break;
        case(2): iot_connect(&home_chan, (Address*)arg, 10); break;
    }
}

void *network_thread(void *eQueue){
    TSUQueue *eventQ = (TSUQueue *)eQueue;
    Event *e = NULL;
    while(1){
        /* Prep Network event */
        e = (Event *) calloc(sizeof(Event), '\0');
        if (e == NULL) {
            printf("Network thread exit, malloc failed\n");
            exit(1);
        }
        e->type = NETWORK_EVENT;
        /* Receive packet */
        while (e->len == 0)
            e->len = net_recvfrom(&(e->pkt), PKT_LEN, e->src, SYNC);
        tsuq_add(eventQ, e); /* Added to event Q */
    }
}


void *ui_thread(void *e){
    TSUQueue *eventQ = (TSUQueue *)e;
    Event *event = NULL;
    char command[10];
    char text[20];
    while(1){
        /* Prep UI event */
        event = (Event *) calloc(sizeof(Event), '\0');
        if (event == NULL) {
            printf("Network thread exit, malloc failed\n");
            exit(1);
        }
        event->type = UI_EVENT;
        /* Get ui event */
        if (fgets(text, sizeof(text), stdin) != NULL){
            if (sscanf(text, "%s", command) == 0) continue;
            if (strncmp(command, "query", 5) == 0){
                event->command = 1;
                if (sscanf(&(text[5]), "%d", (int*)event->arg) == 0)continue;
                printf("Query for device type %d\n", (int)event->arg[0]); 
            }
            if (strncmp(command, "connect", 7) == 0){
                event->command = 2;
                if (net_aton(&(text[7]), (Address*)event->arg) == 0) continue;
                printf("Connect to device %s\n", net_ntoa((Address *)event->arg)); 
            }
        }
        tsuq_add(eventQ, event); /* Added to event Q */
    }
}


int main(){
    pthread_t network_t;
    pthread_t ui_t;
    Event *event = NULL;
    TSUQueue *eventQ = tsuq_create();
    ctable_init_table();
    cstate_init_state(&home_chan, 0);
    //set timer for cleaner(TICK_RATE);
    src = net_addralloc();
    int status = net_init();
    if (status == 0){
        printf("Catastrophic error, exiting.\n");
        return -1;
    }
    pthread_create(&network_t, NULL, network_thread, eventQ);
    pthread_create(&ui_t, NULL, ui_thread, eventQ);
    while (1){
        tsuq_take(eventQ, (void**)&event);
        printf("Event status: %d\n", event->type);
        switch(event->type){
            case(NETWORK_EVENT): network_handler(event->src, event->pkt, event->len); break;
            case(UI_EVENT): ui_handler(event->command, event->arg); break;
            case(HWDB_EVENT): break;
        }
        free(event);
    }
    return 0;
}