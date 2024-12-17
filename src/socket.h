#ifndef H_SOCKET
#define H_SOCKET

#include "include.h"

// initialize tcp and upd socket connections
void sockInit(void);

void sockSend(const void *data, const int size);

// true if recv returns value greater than 0
bool sockRecieve(void *buff, const int size);

#endif