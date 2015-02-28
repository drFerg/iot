#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "ctable.h"

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTFFLUSH(...) printfflush()
#elif SIM
#define PRINTF(...) dbg("DEBUG",__VA_ARGS__)
#define PRINTFFLUSH(...)
#else  
#define PRINTF(...)
#define PRINTFFLUSH(...)
#endif

static Channel *channels;
pthread_mutex_t *lock;
static Channel *freeList;
uint32_t size;
uint32_t alloc_size;
uint32_t next_slot;

void init_state(Channel *chan, uint8_t chan_num){
	chan->state->chan_num = chan_num;
	chan->state->seqno = 0;
	chan->state->remote_addr = 0;
	chan->state->ticks_till_ping = 0;
}
/* 
 * initialise the channel table 
 */
void ctable_init_table(){
	alloc_size = CHANNEL_NUM;
	channels = (Channel *) (malloc(sizeof(Channel) * alloc_size));
	if (channels != NULL) {
		size = 0;
		freeList = NULL;
		lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
		if (lock != NULL) {
			pthread_mutex_init(lock, NULL);
			PRINTF("ctable>> Table create successfully (%d channels)\n", CHANNEL_NUM);
			return;
		}
		else free(channels);
	}
	PRINTF("ctable>> Table create failed\n");
}

/* resize channel table, if possible 
 * return 1 if successful, 0 otherwise
 */
int resize() {
	Channel *new = NULL;
	alloc_size *= 2;
	new = (Channel *) malloc(sizeof(Channel) * alloc_size);
	if (new != NULL) { 
		memcpy(new, channels, size);
		free(channels);
		channels = new;
		PRINTF("ctable>> Table resized (%d channels)\n", alloc_size);
		return 1;
	}
	return 0;
}

/*
 * create a new channel if space available
 * return channel if successful, NULL otherwise
 */
ChanState* ctable_new_channel(){
	Channel *chan = NULL;
	uint32_t chan_num = 0;
	pthread_mutex_lock(lock);
	/* Check if free list has any */
	if (freeList) { 
		chan = freeList;
		freeList = freeList->next;
		chan_num = chan->state->chan_num;
	}
	else { /* Otherwise get next slot in the array */
		/* Check size and resize if necessary */
		if (size >= alloc_size && resize() == 0){
			pthread_mutex_unlock(lock);
			return NULL;
		}
		chan_num = next_slot;
		chan = &(channels[next_slot++]);
		chan->state = (ChanState *) malloc(sizeof(ChanState));
		if (chan->state == NULL) {
			next_slot--;
			pthread_mutex_unlock(lock);
			return NULL;
		}
	}
	init_state(chan, chan_num);
	pthread_mutex_unlock(lock);
	PRINTF("ctable>> Added new channel (%d)\n", chan_num);
	return chan->state;
}

/* 
 * get the channel state for the given channel number
 * return channel if successful, NULL otherwise
 */
ChanState* ctable_get_channel_state(uint32_t chan_num){
	Channel *chan = NULL;
	pthread_mutex_lock(lock);
	chan = &(channels[chan_num]);
	pthread_mutex_unlock(lock);
	return chan->state;
}

/*
 * remove specified channel state from table
 * (adds channel back to free list)
 */
void ctable_remove_channel(uint32_t chan_num){
	pthread_mutex_lock(lock);
	channels[chan_num].next = freeList;
	freeList = &(channels[chan_num]);
	pthread_mutex_unlock(lock);
	PRINTF("ctable>> Removed channel\n");
}

/* 
 * destroys table 
 */
void ctable_destroy_table(){
	pthread_mutex_lock(lock);
	uint32_t i = 0;
	for (; i < next_slot; i++) {
		free(channels[i].state);
	}
	free(channels);
	pthread_mutex_unlock(lock);
	free(lock);
	PRINTF("ctable>> Table destroyed\n");
}