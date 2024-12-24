#ifndef H_TYPES
#define H_TYPES

#include "include.h"
#include "socket.h"

#define BROADCAST_THREAD UINT32_MAX

// type of message object (8-bit)
typedef uint8_t msgType_t;

typedef struct {
    uint8_t   mmver;          // Protocol version (should always be set to <<MMVER>>)
    time_t    tm_timestamp;   // time when message was CREATED!
    msgType_t uc_type;        // type of the message (what it holds)

    uint32_t u32_thread;    // Changed channels to threads (sounds cooler)
                            // when its BROADCAST_THREAD its a broadcasted message

    struct {
        wchar_t wcs_username[MAX_USERNAME]; // who send the message
        wchar_t wcs_address[MAX_USERNAME];  // When NULL it means '*'
        wchar_t wcs_body[MAX_BODY];
    } contents;
    
    BYTE pad[4]; // DONT ACCESS (aligns sizeof(message_t) to 288)

    uint32_t u32_checksum;      /* this field HAS to be at the end so that crc32() call is easier [len(msg) - len(u32)] */
} message_t;


/* Low-level functions */

// raw version of msg_create
message_t *msg_init(void);

// initializes and returns allocated message with specified params
message_t *msg_create(
    const msgType_t type,
    const uint32_t thread
);

// sets internal contents of the message (including username)
void msg_setContent(
    message_t *restrict msg,
    const wchar_t *addr,
    const wchar_t *body
);

/* High-level functions */

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

#endif