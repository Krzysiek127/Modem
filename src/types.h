#ifndef H_TYPES
#define H_TYPES

#include "include.h"

#define MMVER   0x10

#define MAX_USERNAME    24
#define MAX_BODY        80
#define MAX_MOTD        160

typedef struct {
    uint8_t mmver;

    uint32_t u32_channel, u32_chmask;
    wchar_t wcs_username[MAX_USERNAME],
            wcs_body[MAX_BODY];
} message_t;
#define FOREGROUND_DEFAULT  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

#endif