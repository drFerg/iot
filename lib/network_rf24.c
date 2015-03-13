#include "network.h"
#include "rf24.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct address{
    uint8_t addr[5];
} Address;

Address address = { .addr = {0xF0, 0xF0, 0xF0, 0xF0, 0xE1}};
Address broadcast = { .addr = {0xF0, 0xF0, 0xF0, 0xF0, 0xFF}};

int net_init(){
    printf("rf24>> Starting network layer...\n");
    uint8_t status = rf24_init_radio("/dev/spidev0.0", 8000000, 25);
    if (status == 0) return 0;
    rf24_resetcfg();
    rf24_enableDynamicPayloads();
    rf24_setAutoAckOnPipe(1, 0);
    rf24_setRXAddressOnPipe((uint8_t*)&address, 1);
    rf24_startListening();
    rf24_printDetails();
    printf("rf24>> Network layer started!\n");
    return 1;
}

int net_sendto(Address *addr, void *payload, int len){
    return rf24_send((uint8_t*)addr, payload, len);
}

int net_recvfrom(void *payload, int len, Address *addr, int block){
    return rf24_recvfrom(payload, len, (uint8_t*)addr, block);
}

char * net_ntoa(Address *addr){
    static char str[ADDR_WIDTH * 4];
#if ADDR_WIDTH == 5
    sprintf(str, "%d.%d.%d.%d.%d", (addr->addr)[0], (addr->addr)[1], 
                                   (addr->addr)[2], (addr->addr)[3], 
                                   (addr->addr)[4]);
#elif ADDR_WIDTH == 4
    sprintf(str, "%d.%d.%d.%d", (addr->addr)[0], (addr->addr)[1], 
                                (addr->addr)[2], (addr->addr)[3]);
#elif ADDR_WIDTH == 3
    sprintf(str, "%d.%d.%d", (addr->addr)[0], (addr->addr)[1], 
                             (addr->addr)[2]);
#endif
    return str;
}

Address *net_addralloc(){
    return (Address*)malloc(sizeof(Address));
}

void net_addrfree(Address *addr){
    free(addr);
}

void net_addrcpy(Address *dst, Address *src){
    memcpy(dst, src, ADDR_WIDTH);
}

Address *net_addr(){
    return &address;
}

Address *net_broadaddr(){
    return &broadcast;
}