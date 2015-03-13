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
#define ADDR_LEN 8

typedef struct address {
    int port;
    char addr_s[ADDR_LEN];
    struct addrinfo *servinfo;
    struct sockaddr *addr;
    socklen_t addrlen;
} Address;

Address address = {.port = 0};
Address broadcast = {.port = 255};
int sock, length, clientlen, n;
struct sockaddr_in server;
struct sockaddr_in client;
struct addrinfo hints, *servinfo, *p;
socklen_t saddr_len; 

int net_init(Address *addr){
    PRINTF("udp>> Starting network layer...\n");
    /* Loop through all addrinfo results, binding to first valid one */
    for(p = addr->servinfo; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype,
                           p->ai_protocol)) == -1) {
            perror("udp>> Error opening socket");
            continue;
        }
        PRINTF("udp>> Socket opened (%d)\n", sock);

        if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("udp>> Error in binding");
            continue;
        }
        break;
    }
    if (p == NULL) {
        PRINTF("udp>> Failed to bind socket\n");
        return 0;
    }
    freeaddrinfo(addr->servinfo);
    PRINTF("udp>> Socket bound to port %d\n", addr->port);
    PRINTF("udp>> Network layer started!\n");
    return 1;
}

void net_close() {
    PRINTF("udp>> Closed network socket\n");
    close(sock);
}

int net_sendto(Address *addr, void *payload, int len){
    return sendto(sock, payload, len, 0, addr->addr, 
                                         addr->addrlen);
}

int net_recvfrom(void *payload, size_t len, Address **address, int block){
    Address *addr = (Address *) malloc(sizeof(Address));
    if (addr == NULL) return -1;
    *address = addr;
    memset(addr, '\0', sizeof(Address));
    addr->addr = (struct sockaddr *) malloc(sizeof(struct sockaddr));
    if (addr->addr == NULL) {
        free(addr); 
        return -1;
    }
    addr->addrlen = sizeof(struct sockaddr);
    memset(addr->addr, '\0', addr->addrlen);
    return recvfrom(sock, payload, len, 0, addr->addr, 
                                           &(addr->addrlen));
}

char *net_ntoa(Address *addr){
    memset(addr->addr_s, '\0', ADDR_LEN);
    struct sockaddr_in *a = (struct sockaddr_in *) (addr->addr);
    sprintf(addr->addr_s, "%hd", a->sin_port);
    return addr->addr_s;
}

int net_aton(char *addr_s, Address *addr){
    return sscanf(addr_s, "%d", &(addr->port));
}

Address *net_addrcpy(Address *src){
    if (src == NULL) return NULL;
    Address *dst = (Address *) malloc(sizeof(Address));
    if (dst == NULL) return NULL;
    memset(dst, '\0', sizeof(Address));
    memcpy(dst, src, sizeof(Address));
    if (src->servinfo == NULL) return dst;
    dst->servinfo = (struct addrinfo *) malloc(sizeof(struct addrinfo));
    if (dst->servinfo == NULL) return NULL;
    memset(dst->servinfo, '\0', sizeof(struct addrinfo));
    memcpy(dst->servinfo, src->servinfo, sizeof(struct addrinfo));
    dst->addr = dst->servinfo->ai_addr;
    dst->addrlen = dst->servinfo->ai_addrlen;
    return dst;
}
Address *net_addralloc(char *addr_s){
    struct addrinfo hints;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    Address *addr = (Address*)malloc(sizeof(Address));
    if (addr) {
        memset(addr, '\0', sizeof(Address));
        net_aton(addr_s, addr);
        if ((rv = getaddrinfo(NULL, addr_s, &hints, &(addr->servinfo))) != 0) {
            PRINTF("udp>> getaddrinfo: %s\n", gai_strerror(rv));
        }
        addr->addr = addr->servinfo->ai_addr;
        addr->addrlen = addr->servinfo->ai_addrlen;
    }
    return addr;
}
void net_addrfree(Address *addr){
    if (addr->servinfo) freeaddrinfo(addr->servinfo);
    free(addr);
}
Address *net_addr(){
    return &address;
}
Address *net_broadaddr(){
    return &broadcast;
}