#ifndef H_SOCKET
#define H_SOCKET

#include "include.h"
#include "discovery.h"
#include "crc.h"
#include <winsock2.h>


// initialize tcp socket connection
void sock_initTCP(const uint32_t address, const uint16_t port);

// initialize udp socket connection
void sock_initUDP(const uint16_t port);

// send data 
void sock_send(const void *data, const int size);

// recieve data to buffer
void sock_recieve(void **buff, const int size);

#endif