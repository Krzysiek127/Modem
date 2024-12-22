#ifndef H_TYPES
#define H_TYPES

#include "include.h"
#include "socket.h"

// type of message object (8-bit)
typedef uint8_t msgType_t;

typedef struct {
    uint8_t   mmver;          // Protocol version (should always be set to <<MMVER>>)
    time_t    tm_timestamp;   // time when message was CREATED!
    msgType_t uc_type;        // type of the message (what it holds)
    uint8_t   uc_flags;

    uint32_t u32_thread;    // Changed channels to threads (sounds cooler)

    struct {
        wchar_t wcs_username[MAX_USERNAME];
        wchar_t wcs_address[MAX_USERNAME];  // When NULL it means '*'
        wchar_t wcs_body[MAX_BODY];
    } contents;
    
    uint32_t u32_argument;
    uint32_t u32_checksum;      /* this field HAS to be at the end so that crc32() call is easier [len(msg) - len(u32)] */
} message_t;

// these will be generalized and refactored to shit later  |
//                                                         V
/* Low-level functions */
message_t *msg_create(void);
message_t *msg_type(message_t **msgptr,     msgType_t type);
message_t *msg_body(message_t **msgptr,     wchar_t *body);
message_t *msg_uarg(message_t **msgptr,     uint32_t arg);
message_t *msg_setFlags(message_t **msgptr, uint8_t flags);
message_t *msg_setThread(message_t **msgptr,uint32_t thread);

// initializes and returns allocated message with specified params
message_t *msg_create2(
    const msgType_t type,
    const uint32_t thread,
    const uint32_t arg,
    const uint8_t flags
);

// sets internal contents of the message (including username)
void msg_setContent(
    message_t *restrict msg,
    const wchar_t *addr,
    const wchar_t *body
);

/* High-level functions */
message_t *msg_sendText(wchar_t *message, wchar_t *address);
void msg_free(message_t *msg);
void msg_sendFile(wchar_t *path);

uint32_t getCurrentThread(void);
void setCurrentThread(uint32_t th);


// socket interaction functions

void msg_send(message_t *msg);
message_t *msg_recieve(void);


/* Message types */

enum {
    MSG_TEXT = 0,
    MSG_DATA_BEGIN,
    MSG_DATA,
    MSG_DATA_END,
    MSG_DATA_ERROR,

    MSG_CONNECT,
    MSG_DISCONNECT,

    MSG_SHUTDOWN
};

/* Message flags */

#define MFLAG_BROADCAST     (1 << 0)
#define MFLAG_PING          (1 << 1)

#endif