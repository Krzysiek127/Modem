#ifndef H_TYPES
#define H_TYPES

#include "include.h"
#include "socket.h"

typedef struct {
    uint8_t mmver;          // Protocol version should always be set to <<MMVER>>
    time_t  tm_timestamp;
    uint8_t uc_type;        // No enum, we should try to minimize bandwith
    uint8_t uc_flags;

    uint32_t u32_thread;    // Changed channels to threads (sounds cooler)

    wchar_t wcs_username[MAX_USERNAME],
            wcs_address[MAX_USERNAME],  // When NULL it means '*'
            wcs_body[MAX_BODY];

    uint32_t u32_argument;
    uint32_t u32_checksum;      /* this field HAS to be at the end so that crc32() call is easier [len(msg) - len(u32)] */
} message_t;

/* Low-level functions */
message_t *msg_create(void);
message_t *msg_type(message_t **msgptr, uint8_t type);
message_t *msg_body(message_t **msgptr, wchar_t *body);
message_t *msg_uarg(message_t **msgptr, uint32_t arg);
message_t *msg_setth(message_t **msgptr, uint32_t thread);
message_t *msg_setflag(message_t **msgptr, uint8_t flags);


/* High-level functions */
message_t *msg_maketext(wchar_t *message, wchar_t *address);
void msg_free(message_t *msg);
message_t *msg_recv(void);
void msg_sendfile(wchar_t *path);

uint32_t get_current_thread(void);
void set_current_thread(uint32_t th);

/* I need to have access to message_t so this declaration isn't in socket.h */
void sck_sendmsg(message_t *msg);

/* Message types */

#define MTYPE_TEXT 0

#define MTYPE_DATA_BEGIN    1
#define MTYPE_DATA          2
#define MTYPE_DATA_END      3
#define MTYPE_DATA_ERROR    4

#define MTYPE_CONNECT       5
#define MTYPE_DXCONNECT     6

#define MTYPE_SHUTDOWN      7

/* Message flags */

#define MFLAG_BROADCAST     (1 << 0)
#define MFLAG_PING          (1 << 1)
#endif