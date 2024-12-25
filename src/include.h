#ifndef H_INCLUDE
#define H_INCLUDE

#include <stdio.h>   // Try to minimize it's use (nothing against it just design decision. Safely use it for debugging)
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h> // I recommend using stdbool for clarity
#include <wchar.h>

#include <time.h>
#include <assert.h>
#include <io.h>

#define WIN32_LEAN_AND_MEAN
#include <locale.h>
#include <shlwapi.h>
#include <ws2tcpip.h>
#include <shobjidl.h>
#include <winsock2.h>
#include <windows.h>

#include "crc.h"

#define MMVER           0x11
#define MAX_USERNAME    24
#define MAX_BODY        80
#define MAX_MOTD        160
#define MAX_TOAST       120

#define FOREGROUND_DEFAULT  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define PRG_NAME    (L"Modem")
#define PRG_VER     (L"0.1")


/*
    FUNCTION NAMING 101 
    pref_name..()
    pref - prefix, describes general operation type, all lowercase.
    name - actual function name in camelCase.
*/

// toast print
void mm_toast(const wchar_t *text);

// formatted version
void mm_toastf(const wchar_t *format, ...);

uint32_t wcstou32(const wchar_t *wcs);
wchar_t* wcs_copy_n(const wchar_t* source, const size_t n);
size_t wcs_scan(const wchar_t *src);
void MessageBoxW_Format(const wchar_t *format, ...);


// Error reporting

void TIRCriticalError(const wchar_t *text);
void TIRCFormatError(int lasterror);

#define TIRCAssert(condition, text) \
do {                                \
    if((condition)) {               \
        TIRCriticalError(text);     \
    }                               \
} while (0)

bool hexdump(const void *addr, const size_t size, uint8_t perLine);


/*
    Generally UDP would be used for server discovery and advertisiment so it needs timeout (Server selection before connecting)
    And TCP is set to non-blocking, the MSDN said after detecting WSAEWOULDBLOCK it should sleep for few miliseconds 
                                    (to avoid massive computer network explosion and global infrastructure collapse)
*/

/*
    Refer to older repo https://github.com/Krzysiek127/Telchat for instructions

    Also don't forget I still *somewhat* want it to be ran off of "ncat -l 2005 --broker"
    (so that it even works on dumb servers without advertising)
    Downside: Cannot keep connected users lists, allocate them an ID for better addressing etc

    Although I am not sure and maybe writing dedicated server would be better
*/

#endif