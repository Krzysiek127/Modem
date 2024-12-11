#include <stdio.h>      // Try to minimize it's use (nothing against it just design decision. Safely use it for debugging)
#include <stdlib.h>
#include <stdint.h>
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

#define MIN(a,b) (((a)<(b))?(a):(b))

#define PRG_NAME    L"Modem"
#define PRG_VER     L"0.1"

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

    Although am not sure and maybe writing dedicated server would be better
*/


void TIRCAssert(int bool, wchar_t *text);
void TIRCriticalError(wchar_t *text);