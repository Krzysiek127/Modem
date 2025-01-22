#include "screen.h"
#include "message.h"
#include "socket.h"
#include "username.h"

#include <getopt.h>

#define OPT_NEW_USER    0x01
#define OPT_TCP_CONNECT 0x02
#define OPT_UDP_CONNECT 0x04

// All-Project globals
wchar_t wcs_currentUser[MAX_USERNAME];

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s [-u] [-f ip:<port> | -b port]\n", argv[0]);
        return 1;
    }

    // cmdl options
    uint8_t opts = 0;

    opterr = 0;
    int c;
    while ((c = getopt(argc, argv, "uf:b:h")) != -1) {
        switch (c) {

            case 'u':
                opts |= OPT_NEW_USER;
                break;
            case 'f': {
                if (opts & OPT_UDP_CONNECT) {
                    fprintf(stderr, "Only 1 connection type can be specified");
                    return 1;
                }
                opts |= OPT_TCP_CONNECT;

                const char *ip = strtok(optarg, ":");
                const char *port = strtok(NULL, ":");

                TIRCAssert(
                    strlen(ip) > 15 || strlen(ip) < 9,
                    L"Incorrect ip address."
                );
                
                sock_initTCP(inet_addr(ip), port == NULL ? DEFAULT_PORT : (uint16_t)atoi(port));
                break;
            }
            case 'b': {
                if (opts & OPT_TCP_CONNECT) {
                    fprintf(stderr, "Only 1 connection type can be specified");
                    return 1;
                }
                opts |= OPT_UDP_CONNECT;

                char port[6];
                strncpy(port, optarg, 6);

                printf("Discovering Modem server [%s@UDP], please wait...", port);
                sock_initUDP((uint16_t)atoi(port));
                break;
            }
            case 'h':
                printf(
                    "Telthar Modem chat client Va1.0 PV:%xH\n\n"
                    "Usage: %s [-u] [-f ip:<port> | -b port]\n"
                    "-u : Creates or switches a user.\n"
                    "-f : Directly connects to specified address. If port not specified. Defaults to 2005.\n"
                    "-b : UDP port used for server discovery. Defaults to 2005.\n",
                    MMVER, argv[0]
                );
                return 0;
            
            default:
                fprintf(stderr, "Usage: %s [-u] [-f ip:<port> | -b port]\n", argv[0]);
                return 1;
        }
    }

    if (opts & OPT_NEW_USER)
        createUser();
    else
        readUser();

    if (!(opts & OPT_UDP_CONNECT) && !(opts & OPT_TCP_CONNECT)) {
        fprintf(stderr, "No connection type was specified\n");
        return 1;
    }

    message_t *conn = msg_create(MSG_CONNECT, BROADCAST_THREAD);
    msg_send(conn);
    msg_free(conn);

    mm_screenInit();
    mm_clearScreen();
    while (1) {
        mm_kbdLine();
        mm_printHeader();
        
        message_t *recv = msg_recieve();
        if (recv != NULL && msg_interpret(recv)) {
            mm_clearScreen();
            mm_scroll(recv);
        }
        
        mm_screenFlush();
    }
    
    safeExit();
}