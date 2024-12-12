#include "socket.h"

static SOCKET skMain, skBroad;

const char ADDR[] = "127.0.0.1";
USHORT TPORT = 2005;
USHORT UPORT = 2005;


void sck_init(void) {
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

    // not sure if this can be changed to normal bool
    BOOL broadcast = TRUE;
    TIRCAssert(
        setsockopt(skBroad, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) != SOCKET_ERROR,
        L"Failed to set broadcast option on UDP socket"
    );

    TIRCAssert( connect(skMain, (SOCKADDR*)&tcp, sizeof(tcp)) != SOCKET_ERROR, L"Failed to establish a connection to host over TCP" );
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
}

SOCKET *sck_getmainsock(void) {
    return &skMain;
}

SOCKET *sck_getbroadsock(void) {
    return &skBroad;
}