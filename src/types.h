#ifndef H_TYPES
#define H_TYPES

#include "include.h"

#define MMVER   0x10

#define MAX_USERNAME    24
#define MAX_BODY        80
#define MAX_MOTD        160

typedef struct {
    uint8_t mmver;      // Protocol version should always be set to <<MMVER>>
    time_t  tm_timestamp;
    uint8_t uc_type;    // Message type (maybe should be changed to enum)

    // Channel so one server could serve many "group chats" and i thought
    // about "channel masks" (something like subnet masks)
    // so you could send a message
    // to many channels at once (don't know if its useful tho)
    uint32_t u32_channel, u32_chmask; 

    wchar_t wcs_username[MAX_USERNAME],
            wcs_body[MAX_BODY];

    uint32_t u32_argument;
} message_t;


// Needs name refactoring ASAP
struct mm_rdp {
    uint8_t mmver;
    enum {
        MRDP_SOLICITATION,  // Sent by the client "Hey! I want to know the servers here!"
        MRDP_ADVERT         // Sent by the server "Hi, am the server! Here's my IP and PORT. Also here's my MOTD (and TODO: who's currently connected)"
    } rdpt;

    wchar_t wc_motd[MAX_MOTD];
    uint32_t ip_addr, ip_port;
};


#define FOREGROUND_DEFAULT  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

#endif