#include "screen.h"

void TIRCAssert(int bool, wchar_t *text) {
    if (!bool)  // Because it's assert
        TIRCriticalError(text);
}

void TIRCriticalError(wchar_t *text) {
    MessageBoxW(NULL, text, PRG_NAME, MB_ICONERROR);

    WSACleanup();
    exit(1);
}
SOCKET skMain, skBroad;     // TCP(2005) and UDP(2007)


char ADDR[] = "127.0.0.1";
USHORT TPORT = 2005, UPORT = 2007;

int main(int argc, char **argv) {
    WSADATA wsa;

    TIRCAssert(WSAStartup( MAKEWORD(2, 2), &wsa ) == 0, L"Failed to initialize Winsock 2.2");
    TIRCAssert((skMain = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET, L"Failed to initialize TCP socket");
    TIRCAssert((skBroad = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) != INVALID_SOCKET, L"Failed to initialize UDP socket");
    
    struct sockaddr_in tcp, udp;
    tcp.sin_family = AF_INET;
    tcp.sin_addr.s_addr = inet_addr(ADDR);
    tcp.sin_port = htons(TPORT);

    udp.sin_family = AF_INET;
    udp.sin_addr.s_addr = INADDR_BROADCAST;
    udp.sin_port = htons(UPORT);

    BOOL broadcast = TRUE;
    TIRCAssert(
        setsockopt(skBroad, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) != SOCKET_ERROR,
        L"Failed to set broadcast option on UDP socket"
    );

    //TIRCAssert( connect(skMain, (SOCKADDR*)&tcp, sizeof(tcp)) != SOCKET_ERROR, L"Failed to establish a connection to host over TCP" );
    //TIRCAssert( bind(skBroad, (struct sockaddr*)&udp, sizeof(udp)) != SOCKET_ERROR, L"Failed to bind UDP port" );

    /* Set non-blocking mode on TCP socket */
    u_long mode = 1;
    ioctlsocket(skMain, FIONBIO, &mode);

    /* Set timeout on UDP socket */
    int timeout = UDP_TIMEOUT;
    if (setsockopt(skBroad, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        TIRCriticalError(L"Failed to set receive timeout");
    }
    if (setsockopt(skBroad, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        TIRCriticalError(L"Failed to set send timeout");
    }

    mm_scrint();
    while (1) {
        int ch;
        switch (ch = mm_kbdin()) {
            case 0:
            case -1:
                break;
            case 27:
                exit(0);
                break;
            default:
                printf("%lc", (wchar_t)ch);
                break;
        }   
        //printf("Doing something!\r");
    }
    return 0;
}