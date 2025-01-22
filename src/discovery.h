#ifndef H_DISCOVERY
#define H_DISCOVERY

#include "defines.h"

typedef struct {
    uint8_t  mmver;
    uint32_t u32_addr;
    uint16_t u16_port;

    wchar_t wcs_welcome[MAX_TOAST];
    uint32_t u32_crc;
} advert_t;

#endif