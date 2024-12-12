#ifndef H_TYPES
#define H_TYPES

#include "include.h"
#include "socket.h"

typedef struct {
    uint8_t mmver;          // Protocol version should always be set to <<MMVER>>
    time_t  tm_timestamp;
    uint8_t uc_type;        // No enum, we should try to minimize bandwith

    uint32_t u32_thread, u32_thmask;    // Changed channels to threads (sounds cooler)

    wchar_t wcs_username[MAX_USERNAME],
            wcs_body[MAX_BODY];

    uint32_t u32_argument;
} message_t;

void msg_sendtext(wchar_t *message);
void msg_free(message_t *msg);
message_t *msg_recv(void);

#define MTYPE_TEXT 0



#endif