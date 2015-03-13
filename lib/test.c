#include "network.h"
#include "stdio.h"

int main(){
    Address *addr = net_addralloc("12345");
    printf("My addr: %s\n", net_ntoa(net_addr()));
    net_init(addr);
    return 1;
}