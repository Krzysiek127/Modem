#ifndef H_SOCKET
#define H_SOCKET

#include "include.h"

void sck_init(void);
void sck_initUDP(u_long UPORT);
void sck_initTCP(u_long ADDR, uint16_t PORT);

SOCKET *sck_getmainsock(void);
#endif