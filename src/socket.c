#include "socket.h"

static SOCKET skMain, skBroad;

const char ADDR[] = "127.0.0.1";

static inline void sock_initUDP(const uint16_t port) {
    TIRCAssert(
        (skBroad = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET,
        L"Failed to initialize UDP socket"
    );

    struct sockaddr_in UDP;
    UDP.sin_family =      AF_INET;
    UDP.sin_addr.s_addr = INADDR_BROADCAST;
    UDP.sin_port =        htons(port);

    TIRCAssert(
        bind(skBroad, (struct sockaddr*)&UDP, sizeof(UDP)) != SOCKET_ERROR,
        L"Failed to bind UDP port"
    );

    /* Set timeout on UDP socket */
    uint8_t timeout = UDP_TIMEOUT;
    bool broadcast = true;

    TIRCAssert(
        setsockopt(skBroad, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) < 0,
        L"Failed to set broadcast option on UDP socket"
    );
    TIRCAssert(
        setsockopt(skBroad, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0,
        L"Failed to set send timeout"
    );
    TIRCAssert(
        setsockopt(skBroad, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0,
        L"Failed to set receive timeout"
    );
}


static inline void sock_initTCP(const uint16_t port) {
    TIRCAssert(
        (skMain = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET,
        L"Failed to initialize TCP socket"
    );

    struct sockaddr_in TCP;
    TCP.sin_family =      AF_INET;
    TCP.sin_addr.s_addr = inet_addr(ADDR);
    TCP.sin_port =        htons(port);

    TIRCAssert(
        connect(skMain, (SOCKADDR*)&TCP, sizeof(TCP)) == SOCKET_ERROR,
        L"Failed to establish a connection to host over TCP"
    );

    // Set non-blocking mode
    u_long mode = 1;
    ioctlsocket(skMain, FIONBIO, &mode);
}


void sock_init(void) {
    WSADATA wsa;

    TIRCAssert(
        WSAStartup(MAKEWORD(2, 2), &wsa) != 0,
        L"Failed to initialize Winsock 2.2"
    );

    sock_initTCP(2005);
    //sock_innitUDP(2005);
}


void sock_send(const void *data, const int size) {

    if (send(skMain, (const char*)data, size, 0) == SOCKET_ERROR)
        TIRCFormatError(WSAGetLastError());
}

bool sock_recieve(void *buff, const int size) {

    if (buff == NULL)
        return false;

    return (recv(skMain, (char *)buff, size, 0) > 0);
}