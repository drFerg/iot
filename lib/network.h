#ifndef NETWORK_H
#define NETWORK_H

typedef struct address Address;

int net_init();
void net_sendto(Address *addr, void *payload, int len);
int net_receivefrom(Address *addr, void *payload, int len, int block);

#endif /* NETWORK_H */