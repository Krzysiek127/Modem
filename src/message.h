#ifndef H_MSG
#define H_MSG

#include "include.h"
#include "socket.h"
#include "crc.h"

#define BROADCAST_THREAD UINT16_MAX

// type of message object (8-bit)
typedef uint8_t msgType_t;

typedef struct {
    uint8_t   mmver;        // Protocol version (should always be set to <<MMVER>>)
    time_t    timestamp;    // time when message was SEND!
    msgType_t type;         // type of the message (what it holds)

    uint16_t  thread;       // when its BROADCAST_THREAD its a broadcasted message, default is 0

    struct {
        wchar_t wcs_username[MAX_USERNAME]; // who send the message
        wchar_t wcs_address[MAX_USERNAME];  // for who this message is (when empty its everyone)
        wchar_t wcs_body[MAX_BODY];
    } contents;
    
    // r.i.p. funny pad...
    //unsigned char padThatOnlyExistsToAlignSizeOfMessageStructTo288BytesForStupidCrc32ToWorkLol[5];

    uint32_t u32_checksum;      /* this field HAS to be at the end so that crc32() call is easier [len(msg) - len(u32)] */
} message_t;

/* Low-level functions */

// raw version of msg_create
message_t *msg_init(void);

// initializes and returns allocated message with specified params
message_t *msg_create(const msgType_t type, const uint16_t thread);

// sets internal contents of the message (including username)
void msg_setContent(message_t *restrict msg, const wchar_t *addr, const wchar_t *body);

void msg_free(message_t *msg);

/* High-level functions */

// analyzes message and returns true if message is printable
bool msg_interpret(message_t *msg);

void msg_sendFile(const wchar_t *path);

uint16_t getCurrentThread(void);
void setCurrentThread(const uint16_t th);


/* socket interaction functions */

void msg_send(message_t *msg);

// Returns NULL when no message was recieved
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