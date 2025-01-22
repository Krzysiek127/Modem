#ifndef DEFINES_H
#define DEFINES_H

// global code defines + stdint header (superior one)
#include <stdint.h>

#define MMVER           0x13u
#define MAX_USERNAME    24u
#define MAX_BODY        80u
#define MAX_TOAST       120u

#define PRG_NAME    (L"Modem")
#define PRG_VER     (L"0.2")


#define DEFAULT_PORT 2005u
#define UDP_TIMEOUT 50
#define TCP_SLEEP   0

// suck my cock windows
#define WIN32_LEAN_AND_MEAN

#endif