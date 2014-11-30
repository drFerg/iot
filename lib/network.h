#ifndef NETWORK_H
#define NETWORK_H
#define SYNC  1
#define ASYNC 0
typedef struct address Address;

int net_init();
int net_sendto(Address *addr, void *payload, int len);
int net_recvfrom(void *payload, int len, Address *addr, int block);
char *net_ntoa(Address *addr);
int net_aton(char *addr_s, Address *addr);
void net_addrcpy(Address *dst, Address *src);
Address *net_addralloc();
void net_addrfree(Address *addr);
Address *net_addr();
Address *net_broadaddr();
#endif /* NETWORK_H */