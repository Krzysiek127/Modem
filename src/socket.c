#include "socket.h"

static SOCKET skMain, skBroad;

void sock_initUDP(const uint16_t port) {
    WSADATA wsa;

    TIRCAssert(
        WSAStartup(MAKEWORD(2, 2), &wsa) != 0,
        L"Failed to initialize Winsock 2.2"
    );


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

    advert_t advert;
    struct sockaddr_in si_other;
    int si_otherSize = sizeof(si_other);

    while (1)
    {
        int recl = recvfrom(skBroad, (char*)&advert, sizeof(advert_t), 0, (struct sockaddr*)&si_other, &si_otherSize);

        if (recl != sizeof(advert_t)) {
            wprintf(L"Received datagram is of invalid length\n");
            continue;
        }
        
        if (advert.u32_crc != crc32(&advert, sizeof(advert_t) - sizeof(uint32_t))) {
            wprintf(L"Invalid checksum\n");
            continue;
        }

        break;
    }

    closesocket(skBroad);
    sock_initTCP(advert.u32_addr, advert.u16_port);

    mm_toast(advert.wcs_welcome);
}


void sock_initTCP(const uint32_t address, const uint16_t port) {
    WSADATA wsa;
    TIRCAssert(
        WSAStartup(MAKEWORD(2, 2), &wsa) != 0,
        L"Failed to initialize Winsock 2.2"
    );


    TIRCAssert(
        (skMain = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET,
        L"Failed to initialize TCP socket"
    );

    struct sockaddr_in TCP;
    TCP.sin_family =      AF_INET;
    TCP.sin_addr.s_addr = address;
    TCP.sin_port =        htons(port);

    TIRCAssert(
        connect(skMain, (SOCKADDR*)&TCP, sizeof(TCP)) == SOCKET_ERROR,
        L"Failed to establish a connection to host over TCP"
    );

    // Set non-blocking mode
    u_long mode = 1;
    ioctlsocket(skMain, FIONBIO, &mode);
}

void sock_send(const void *data, const int size) {

    if (send(skMain, (const char*)data, size, 0) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK)
            TIRCFormatError(err);
        Sleep(TCP_SLEEP);
    }
}

bool sock_recieve(void *buff, const int size) {

    if (buff == NULL)
        return false;

    return (recv(skMain, (char *)buff, size, 0) > 0);
}