/* A dummy network module for testing on machines without rf24 access */
#include "network.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#define PRINTFFLUSH(...)
#endif

typedef struct address {
    int addr;
} Address;

Address address = {.addr = 0};
Address broadcast = {.addr = 255};
int sock, length, clientlen, n;
struct sockaddr_in server;
struct sockaddr_in client;
socklen_t saddr_len; 

int net_init(Address *addr){
    PRINTF("dummy_udp>> Starting network layer...\n");
    address.addr = addr->addr;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) perror("dummy_udp>> Error in opening socket");
    PRINTF("dummy_udp>> Socket opened (%d)\n", sock);
    length = sizeof(struct sockaddr_in);
    memset(&server, 0, length);
    saddr_len = sizeof(struct sockaddr_in);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(address.addr);
    if (bind(sock, (struct sockaddr *) &server, length) < 0)
        perror("dummy_udp>> Error in binding");
    /* spin off sensor/actuator threads? */
    PRINTF("dummy_udp>> Socket bound to port %d\n", address.addr);
    PRINTF("dummy_udp>> Network layer started!\n");
    return 1;
}

void net_close() {
    PRINTF("dummy_udp>> Closed network socket\n");
    close(sock);
}

int net_sendto(Address *addr, void *payload, int len){
    return sendto(sock, payload, len, 0, (struct sockaddr *) &client, saddr_len);
}

int net_recvfrom(void *payload, size_t len, Address *addr, int block){
    return recvfrom(sock, payload, len, 0, (struct sockaddr *) &client, &saddr_len);
}

char * net_ntoa(Address *addr){
    static char str[4];
    sprintf(str, "%d", addr->addr);
    return str;
}
int net_aton(char *addr_s, Address *addr){
    return sscanf(addr_s, "%d", &(addr->addr));
}

void net_addrcpy(Address *dst, Address *src){
    dst->addr = src->addr;
}
Address *net_addralloc(char *addr_s){
    Address *addr = (Address*)malloc(sizeof(Address));
    if (addr) net_aton(addr_s, addr);
    return addr;
}
void net_addrfree(Address *addr){
    free(addr);
}
Address *net_addr(){
    return &address;
}
Address *net_broadaddr(){
    return &broadcast;
}