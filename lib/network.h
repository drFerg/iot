#ifndef NETWORK_H
#define NETWORK_H

typedef struct address Address;

int net_init();
void net_sendto(Address *addr, void *payload, int len);
int net_receivefrom(Address *addr, void *payload, int len, int block);
char * net_ntoa(Address *addr);
Address *net_addr();
#endif /* NETWORK_H */