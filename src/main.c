#include "screen.h"
#include "message.h"

// All-Project globals
wchar_t wcs_current_user[MAX_USERNAME] = {0};
FILE *fUserFile;

int main(int argc, char **argv) {
    mm_scrint();
    mm_clearscr();


    if (access("username", F_OK) == 0) {
        fUserFile = fopen("username", "rb");
        
        fseek(fUserFile, 0L, SEEK_END);
        size_t fSize = ftell(fUserFile);
        if (fSize != MAX_USERNAME * sizeof(wchar_t))
            TIRCriticalError(L"Invalid username stored! Delete the file");
        
        rewind(fUserFile);
        fread(wcs_current_user, sizeof(wchar_t), MAX_USERNAME, fUserFile);
        fclose(fUserFile);
    } else {
username_register:
        wprintf(L"No username file!\nNew username> ");
        wchar_t *wcs_usrin = mm_kbdraw(MAX_USERNAME);
        wmemcpy(wcs_current_user, wcs_usrin, MAX_USERNAME);
        free(wcs_usrin);

        BOOL removeRest = FALSE;
        for (wchar_t *wcsptr = wcs_current_user; *wcsptr; wcsptr++) {
            switch (*wcsptr) {
            case 0:
            case L'\r':
            case L' ':
            case L'\n':
            case L'\t':
                /* From this point onwards remove every char */
                *wcsptr = 0;
                removeRest = TRUE;
                break;
            default:
                if (removeRest || !iswprint(*wcsptr))
                    *wcsptr = 0;
                break;
            }
        }

        if (*wcs_current_user == 0) {
            MessageBoxW(NULL, L"Invalid username!", L"Name error", 0);
            goto username_register;
        }
        
        fUserFile = fopen("username", "wb");
        fwrite(wcs_current_user, sizeof(wchar_t), MAX_USERNAME, fUserFile);
        fclose(fUserFile);
    }
    
    sck_init();

    u_long broadcast_port = DEFAULT_PORT;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "/f")) {
            char *ip = strtok(argv[i+1], ":");
            char *port = strtok(NULL, ":");
            sck_initTCP(inet_addr(ip), port == NULL ? DEFAULT_PORT : atoi(port));
            
            goto end_argv_check;
        }
        if (!strcmp(argv[i], "/b")) {
            broadcast_port = atoi(argv[i+1]);
            break;
        }
        if (!strcmp(argv[i], "/h")) {
            printf("Telthar Modem chat client V1.0 PV:%xH\n\nUsage: %s [/f ip<:port>] [/b port]\n\t/f ip<:port>\tDirect connection to specified address. If port not specified, it defaults to 2005. Skips UDP server discovery.\n\t/b port\t\tUDP port used for server discovery. Defaults to 2005.\n\n",
                    MMVER, strrchr(argv[0], '\\') + 1
            );
            return 0;
        }
    }
    printf("Discovering Modem server [%hu@UDP], please wait...", broadcast_port);
    sck_initUDP(broadcast_port);  // We didn't encounter `/f ip<:port>` so we need to await broadcasts
    end_argv_check:
    mm_curvis(FALSE);

    message_t *conn = msg_create();
    msg_type(&conn, MTYPE_CONNECT);
    msg_setflag(&conn, MFLAG_BROADCAST);
    msg_setth(&conn, get_current_thread());
    sck_sendmsg(conn);

    mm_clearscr();
    while (1) {
        mm_kbdline();
        mm_printlbuf();
        message_t *recv = msg_recv();
        if (recv != NULL) {
            mm_clearscr();
            mm_scroll(recv);
        }
        
        mm_scrflush();
    }
    return 0;
}