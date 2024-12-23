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

    TIRCAssert(
        fUserFile == NULL,
        L"Username file could not be read"
    );
    
    fseek(fUserFile, 0L, SEEK_END);

    TIRCAssert(
        ftell(fUserFile) != (MAX_USERNAME * sizeof(wchar_t)),
        L"Invalid username stored! Delete the file"
    );
    
    rewind(fUserFile);
    fread(wcs_current_user, sizeof(wchar_t), MAX_USERNAME, fUserFile);
    fclose(fUserFile);

    return false;
}


int main(void) {
    if (inspectUserFile()) {
        wprintf(L"No username file!\nNew username> ");

        TIRCAssert(
            fgetws(wcs_current_user, MAX_USERNAME, stdin) == NULL,
            L"Invalid username"
        );
        
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

    mm_screenInit();
    mm_screenClear();

    message_t *conn = msg_create(MSG_CONNECT, getCurrentThread(), 0, MFLAG_BROADCAST);
    msg_send(conn);
    msg_free(conn);

    while (1) {
        mm_kbdLine();
        mm_printLineBuff();
        
        message_t *recv = msg_recieve();
        if (recv != NULL) {
            mm_screenClear();
            mm_scroll(recv);
        }
        
        mm_screenFlush();
    }
    return 0;
}