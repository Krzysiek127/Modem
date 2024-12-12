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


#define MMVER   0x10
#define MAX_USERNAME    24
#define MAX_BODY        80
#define MAX_MOTD        160
#define FOREGROUND_DEFAULT  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define PRG_NAME    (L"Modem")
#define PRG_VER     (L"0.1")

#define UDP_TIMEOUT 50
#define TCP_SLEEP   100
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

void TIRCAssert(bool condition, const wchar_t *text);
void TIRCriticalError(const wchar_t *text);
void TIRCFormatError(int lasterror);

#endif