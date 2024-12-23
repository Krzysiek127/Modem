#include "../src/discovery.h"
#include "../src/crc.h"

#include <winsock2.h>
WSADATA wsa;

advert_t advert = {
    .mmver = MMVER,
    .u16_port = 2005
};

int main(int argc, char **argv) {
    setlocale(LC_ALL, ".UTF8");
    wcscpy(advert.wcs_welcome, L"Telthar Modem client");

    char *ip_addr = NULL;
    DWORD dInterval = 1000;
    u_long BRDPORT = DEFAULT_PORT;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "/p")) {
            advert.u16_port = atoi(argv[++i]);
        }
        if (!strcmp(argv[i], "/P")) {
            BRDPORT = atoi(argv[++i]);
        }
        if (!strcmp(argv[i], "/t")) {
            dInterval = atoi(argv[++i]);
            dInterval = dInterval < 100 ? 100 : dInterval;  // Anything below 100ms could be too unstable/network intensive
        }
        if (!strcmp(argv[i], "/i")) {
            ip_addr = argv[++i];
        }
        if (!strcmp(argv[i], "/m")) {
            printf("Enter custom welcome> ");
            fgetws(advert.wcs_welcome, MAX_TOAST, stdin);
            for (wchar_t *p = advert.wcs_welcome; *p; p++) {
                if (!isprint(*p))
                    *p = 0;
            }
            printf("\n");
        }

        if (!strcmp(argv[i], "/h")) {
            printf("Modem chat server broadcaster\nUsage: %s [/p port] [/t interval] [/i interface] [/m]\n\n\t/p port\t\tChange server port\n\t/P port\t\tChange broadcasting port\n\t/t\t\tChange broadcasting interval (in ms)\n\t/i ip_addr\tUse selected interface\n\t/m\t\tPrompt for welcome message\n\n", 
                    strrchr(argv[0], '\\') + 1
            );
            return 0;
        }
    }
    WSAStartup(MAKEWORD(2, 2), &wsa);
    
    if (ip_addr == NULL) {
        char hostname[256];
        struct hostent* host_info;
        struct in_addr addr;

        // Get the hostname
        if (gethostname(hostname, sizeof(hostname)) != 0) {
            printf("gethostname failed: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Get host information
        host_info = gethostbyname(hostname);
        if (host_info == NULL) {
            printf("gethostbyname failed: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        advert.u32_addr = *(u_long*)host_info->h_addr_list[0];
    } else {
        advert.u32_addr = inet_addr(ip_addr);
    }

    advert.u32_crc = crc32(&advert, sizeof(advert_t) - sizeof(uint32_t));

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    char broadcast = '1';
    if(setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast)) < 0) {
        printf("Could not set broadcast opt!\n");
        return 0;
    }

    struct sockaddr_in Recv_addr;
    struct sockaddr_in Sender_addr;

    int len = sizeof(struct sockaddr_in);
    Recv_addr.sin_family       = AF_INET;        
    Recv_addr.sin_port         = htons(BRDPORT);   
    Recv_addr.sin_addr.s_addr  = INADDR_BROADCAST;

    printf("Broadcasting on %u! Now run 'ncat -l %u --broker' to start the actual server.\nInfo:\n\tInterface: %s\n\tServer port: %u\n\tWelcome: \"%ls\"\n\tInterval: %u ms\n\n", 
        DEFAULT_PORT, advert.u16_port, inet_ntoa(*(struct in_addr*)&advert.u32_addr), advert.u16_port, advert.wcs_welcome, dInterval
    );
    while (1) {
        sendto(sock, (char*)&advert, sizeof(advert_t), 0, (struct sockaddr *)&Recv_addr, sizeof(Recv_addr));
        Sleep(dInterval);
    }
}