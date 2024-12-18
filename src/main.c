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

    if (ftell(fUserFile) != MAX_USERNAME * sizeof(wchar_t))
        TIRCriticalError(L"Invalid username stored! Delete the file");
    
    rewind(fUserFile);
    fread(wcs_current_user, sizeof(wchar_t), MAX_USERNAME, fUserFile);
    fclose(fUserFile);

    return false;
}


int main(void) {
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

    sock_init();
    mm_cursorVis(false);

    mm_screenInit();
    mm_screenClear();

    message_t *conn = msg_create();
    msg_type(&conn, MSG_CONNECT);
    msg_setFlags(&conn, MFLAG_BROADCAST);
    msg_setThread(&conn, getCurrentThread());

    msg_send(conn);

    while (1) {
        mm_kbdLine();
        mm_printLbuf();
        message_t *recv = msg_recieve();
        if (recv != NULL) {
            mm_screenClear();
            mm_scroll(recv);
        }
        
        mm_screenFlush();
    }
    return 0;
}