#ifndef CHANNEL_TABLE
#define CHANNEL_TABLE
#include <stdint.h>
#include "cstate.h"
/* Num of channels available in table */
#ifndef CHANNEL_NUM
#define CHANNEL_NUM 5
#endif /* CHANNEL_NUM */

typedef struct knot_channel{
	ChanState state;
	struct knot_channel *nextChannel;
	uint8_t active;
}Channel;

/* 
 * initialise the channel table 
 */
void ctable_init_table();

/*
 * create a new channel if space available
 * return channel if successful, NULL otherwise
 */
ChanState * ctable_new_channel();

/* 
 * get the channel state for the given channel number
 * return 1 if successful, 0 otherwise
 */
ChanState * ctable_get_channel_state(int channel);

/*
 * remove specified channel state from table
 * (scrubs and frees space in table for a new channel)
 */
void ctable_remove_channel(int channel);

/* 
 * destroys table 
 */
void ctable_destroy_table();
#endif