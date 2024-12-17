#include "screen.h"
#include "message.h"

#define fileExists(fileName) (_waccess((fileName), F_OK))

// All-Project globals
wchar_t wcs_current_user[MAX_USERNAME];

// returns 0(false) on success
static inline bool inspectUserFile(void) {
    if (fileExists(L"username"))
        return true;

    FILE *fUserFile = fopen("username", "rb");

    if (fUserFile == NULL)
        TIRCriticalError(L"Username file could not be read");
    
    fseek(fUserFile, 0L, SEEK_END);
    const size_t fSize = ftell(fUserFile);

    if (fSize != MAX_USERNAME * sizeof(wchar_t))
        TIRCriticalError(L"Invalid username stored! Delete the file");
    
    rewind(fUserFile);
    fread(wcs_current_user, sizeof(wchar_t), MAX_USERNAME, fUserFile);
    fclose(fUserFile);

    return false;
}


int main(int argc, char **argv) {
    if (inspectUserFile()) {
        wprintf(L"No username file!\nNew username> ");

        if (fgetws(wcs_current_user, MAX_USERNAME, stdin) == NULL)
            TIRCriticalError(L"Invalid username");
        
        // remove ungraph-able chars (if there are any)
        for (wchar_t *wcsptr = wcs_current_user; *wcsptr; wcsptr++) {
            if (!iswgraph(*wcsptr))
                *wcsptr = '\0';
        }

        FILE *fUserFile = fopen("username", "wb");
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