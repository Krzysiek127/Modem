#include "socket.h"
#include "discovery.h"

static SOCKET skMain;
static struct sockaddr_in tcp;
static WSADATA wsa;

void sck_init(void) {
    if (WSAStartup( MAKEWORD(2, 2), &wsa ) != 0)
        TIRCriticalError(L"Failed to initialize Winsock 2.2");
}

void sck_initUDP(u_long UPORT) {
    struct sockaddr_in si_this, si_other;
    int si_otherlen = sizeof(si_other);
    SOCKET broadcast;

    if ((broadcast = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        TIRCriticalError(L"Invalid UDP socket!");
    
    si_this.sin_family = AF_INET;
    si_this.sin_addr.s_addr = INADDR_ANY;
    si_this.sin_port = htons(UPORT);

    if ((bind(broadcast, (struct sockaddr*)&si_this, sizeof(si_this)) == SOCKET_ERROR)) {
        TIRCriticalError(L"Cannot bind UDP socket");
    }

recv_advert:
    advert_t advert;
    int recl = recvfrom(broadcast, (char*)&advert, sizeof(advert_t), 0, (struct sockaddr*)&si_other, &si_otherlen);
    if (recl != sizeof(advert_t)) {
        wprintf(L"Received datagram is of invalid length\n");
        goto recv_advert;
    }
    
    if ( advert.u32_crc != crc32(&advert, sizeof(advert_t) - sizeof(uint32_t)) ) {
        wprintf(L"Invalid checksum\n");
        goto recv_advert;
    }
    closesocket(broadcast);
    sck_initTCP(advert.u32_addr, advert.u16_port);
    mm_toast(advert.wcs_welcome);
}


void sck_initTCP(u_long ADDR, uint16_t PORT) {
    if ((skMain = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        TIRCriticalError(L"Failed to initialize TCP socket");
    }

    tcp.sin_family = AF_INET;
    tcp.sin_addr.s_addr = ADDR;
    tcp.sin_port = htons(PORT);

    if (connect(skMain, (SOCKADDR*)&tcp, sizeof(tcp)) == SOCKET_ERROR) {
        TIRCriticalError(L"Failed to establish a connection to host over TCP");
    }
    u_long mode = 1;
    ioctlsocket(skMain, FIONBIO, &mode);
}

SOCKET *sck_getmainsock(void) {
    return &skMain;
}