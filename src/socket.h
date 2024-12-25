#ifndef H_SOCKET
#define H_SOCKET

#include "include.h"
#include "discovery.h"

#define DEFAULT_PORT (uint16_t)(2005)

#define UDP_TIMEOUT 50
#define TCP_SLEEP   0

// initialize tcp socket connection
void sock_initTCP(const uint32_t address, const uint16_t port);

// initialize udp socket connection
void sock_initUDP(const uint16_t port);

// send data 
void sock_send(const void *data, const int size);

// returns true on success
bool sock_recieve(void *buff, const int size);

#endif