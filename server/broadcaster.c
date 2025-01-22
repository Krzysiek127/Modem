#include "../src/discovery.h"
#include "../src/crc.h"

#include <stdio.h>
#include <getopt.h>
#include <winsock2.h>
#include <locale.h>

int main(int argc, char **argv) {
    setlocale(LC_ALL, ".UTF8");

        advert_t advert = (advert_t){
        .mmver = MMVER,
        .u16_port = DEFAULT_PORT
    };

    wcscpy(advert.wcs_welcome, L"Telthar Modem client.");

    char *ip_addr      = NULL;
    uint32_t dInterval = 1000;
    uint16_t BRDPORT   = DEFAULT_PORT;

    int c;
    while ((c = getopt(argc, argv, "p:P:i:t:mh")) != -1) {
        switch (c) {

            case 'p':
                advert.u16_port = atoi(optarg);
                break;
            case 'P':
                BRDPORT = atoi(optarg);
                break;
            case 't':
                dInterval = atoi(optarg);
                dInterval = dInterval < 100 ? 100 : dInterval;  // Anything below 100ms could be too unstable/network intensive
                break;
            case 'i': 
                ip_addr = optarg;
                break;
            case 'm':
                printf("Enter custom welcome: ");
                fgetws(advert.wcs_welcome, MAX_TOAST, stdin);
                for (wchar_t *p = advert.wcs_welcome; *p; p++) {
                    if (!iswprint(*p))
                        *p = 0;
                }
                putchar('\n');
                break;
            case 'h':
                printf(
                    "Modem chat server broadcaster PV:%xH\n\n"
                    "Usage: %s [-m] [-p port] [-P port] [-t interval] [-i interface]\n"
                    "-p : Change server port. Defaults to 2005\n"
                    "-P : Change broadcasting port. Defaults to 2005\n"
                    "-t : Change broadcasting interval (in miliseconds). Defaults to 1 second\n"
                    "-i : Use selected interface\n"
                    "-m : Prompt for custom welcome message\n", 
                    MMVER, argv[0]
                );
                return 0;
            
            default:
                fprintf(stderr, "Usage: %s [-m] [-p port] [-P port] [-t interval] [-i interface]\n", argv[0]);
                return 1;
        }
    }

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    
    if (ip_addr == NULL) {
        char hostname[256];

        // Get the hostname
        if (gethostname(hostname, sizeof(hostname))) {
            printf("gethostname failed: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Get host information
        struct hostent* host_info = gethostbyname(hostname);
        if (host_info == NULL) {
            printf("gethostbyname failed: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        advert.u32_addr = *((uint32_t*)(*host_info->h_addr_list));
    } 
    else
        advert.u32_addr = inet_addr(ip_addr);

    // crc32 check
    advert.u32_crc = crc32(&advert, sizeof(advert_t) - sizeof(uint32_t));

    SOCKET sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    const char broadcast = '1';
    if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        printf("Could not set broadcast opt!\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    struct sockaddr_in Recv_addr;

    Recv_addr.sin_family       = AF_INET;
    Recv_addr.sin_addr.s_addr  = INADDR_BROADCAST;
    Recv_addr.sin_port         = htons(BRDPORT);

    printf(
        "Broadcasting on %hu! Now run 'ncat -l %hu --broker' to start the actual server.\n"
        "Info:\n"
        "\tInterface:   %s\n"
        "\tServer port: %u\n"
        "\tWelcome:     \"%ls\"\n"
        "\tInterval:    %u ms\n\n",
        DEFAULT_PORT,
        advert.u16_port,
        inet_ntoa(*(struct in_addr*)&advert.u32_addr), // can cause ub, changing later
        advert.u16_port,
        advert.wcs_welcome,
        dInterval
    );

    while (1) {
        sendto(sock, (char*)&advert, sizeof(advert_t), 0, (struct sockaddr*)&Recv_addr, sizeof(Recv_addr));
        Sleep(dInterval);
    }
    
    closesocket(sock);
    WSACleanup();

    return 0;
}