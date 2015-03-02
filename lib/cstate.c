/*
* author Fergus William Leahy
*/
#include "cstate.h"
#include "stdlib.h"
void cstate_init_state(ChanState *state, uint8_t chan_num){
    //state->remote_addr = net_addralloc();
    state->chan_num = chan_num;
    state->seqno = 0;
    state->remote_addr = 0;
    state->ticks_till_ping = 0;
}