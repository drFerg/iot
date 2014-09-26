#include "network.h"
#include "rf24.h"
#include "stdio.h"
typedef struct address{
    uint8_t addr[5];
} Address;

Address address = { .addr = {0xF0, 0xF0, 0xF0, 0xF0, 0xE1}};

int net_init(){
    uint8_t status = rf24_init_radio("/dev/spidev0.0", 8000000, 25);
    if (status == 0) return 0;
    rf24_resetcfg();
    rf24_enableDynamicPayloads();
    rf24_setAutoAckOnPipe(1, 0);
    rf24_setRXAddressOnPipe((uint8_t*)&address, 1);
    rf24_startListening();
    rf24_printDetails();
    return 1;
}

void net_sendto(Address *addr, void *payload, int len){
    rf24_send((uint8_t*)addr, payload, len);
}

int net_receivefrom(Address *addr, void *payload, int len, int block){
    return rf24_recvfrom(payload, len, (uint8_t*)addr, block);
}

char * net_ntoa(Address *addr){
    static char str[ADDR_WIDTH * 4];
    switch(ADDR_WIDTH){
        case(5): sprintf(str, "%d.%d.%d.%d.%d", (addr->addr)[0], (addr->addr)[1], 
                                                (addr->addr)[2], (addr->addr)[3], 
                                                (addr->addr)[4]);break;
        case(4): sprintf(str, "%d.%d.%d.%d", (addr->addr)[0], (addr->addr)[1], 
                                             (addr->addr)[2], (addr->addr)[3]); break;
        case(3): sprintf(str, "%d.%d.%d", (addr->addr)[0], (addr->addr)[1], 
                                          (addr->addr)[2]); break;
    }
    return str;
}

Address *net_addr(){
    return &address;
}