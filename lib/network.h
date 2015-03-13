#ifndef NETWORK_H
#define NETWORK_H
#define SYNC  1
#define ASYNC 0
#include <stddef.h>
typedef struct address Address;

int net_init(Address *addr);
void net_close();
int net_sendto(Address *addr, void *payload, int len);
int net_recvfrom(void *payload, size_t len, Address **address, int block);
char *net_ntoa(Address *addr);
int net_aton(char *addr_s, Address *addr);
Address *net_addrcpy(Address *src);
Address *net_addralloc(char *addr_s);
void net_addrfree(Address *addr);
Address *net_addr();
Address *net_broadaddr();
#endif /* NETWORK_H */