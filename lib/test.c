#include "network.h"
#include "stdio.h"

int main(){
    printf("My addr: %s\n", net_ntoa(net_addr()));
    net_init();
    return 1;
}