#ifndef H_INCLUDE
#define H_INCLUDE

#include "defines.h"
#include <stdio.h>   // Try to minimize it's use (nothing against it just design decision. Safely use it for debugging)
#include <stdlib.h>
#include <stdbool.h> // I recommend using stdbool for clarity
#include <wchar.h>

#include <locale.h>
#include <time.h>

#include <windows.h>
#include <shlwapi.h>

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

// wide char string to uint32_t
uint32_t wcstou32(const wchar_t *wcs);

// memcpy but it allocates a buffer for it
void *memcpy_n(const void *restrict source, const size_t size);

// wcslen but it ignores whitespace
size_t wcs_scan(const wchar_t *src);

// formatted messagebox
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

// safely terminates program duhh...
void safeExit(void);

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