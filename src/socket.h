#ifndef H_SOCKET
#define H_SOCKET

#include "include.h"

void sck_init(void);
SOCKET *sck_getmainsock(void);
SOCKET *sck_getbroadsock(void);

#endif