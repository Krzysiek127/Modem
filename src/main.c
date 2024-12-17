#include "screen.h"
#include "message.h"

// All-Project globals
wchar_t wcs_current_user[MAX_USERNAME];
FILE *fUserFile;

int main(int argc, char **argv) {
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
        wprintf(L"No username file!\nNew username> ");
        if (fgetws(wcs_current_user, MAX_USERNAME, stdin) == NULL)
            TIRCriticalError(L"Invalid username");
        

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
        fUserFile = fopen("username", "wb");
        fwrite(wcs_current_user, sizeof(wchar_t), MAX_USERNAME, fUserFile);
        fclose(fUserFile);
    }

    //wcscpy(wcs_current_user, L"Krzysiek");
    sck_init();
    mm_curvis(FALSE);

    mm_scrint();
    mm_clearscr();

    message_t *conn = msg_create();
    msg_type(&conn, MTYPE_CONNECT);
    msg_setflag(&conn, MFLAG_BROADCAST);
    msg_setth(&conn, get_current_thread());
    sck_sendmsg(conn);

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