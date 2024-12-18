#ifndef H_SOCKET
#define H_SOCKET

#include "include.h"

// initialize tcp and upd socket connections
void sock_init(void);

void sock_send(const void *data, const int size);

// returns true on success
bool sock_recieve(void *buff, const int size);

#endif