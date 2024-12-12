#include "screen.h"

void TIRCAssert(bool condition, const wchar_t *text) {
    if (!condition)  // Because it's assert
        TIRCriticalError(text);
}

void TIRCriticalError(const wchar_t *text) {
    MessageBoxW(NULL, text, PRG_NAME, MB_ICONERROR);
    
    WSACleanup();
    exit(1);
}

void TIRCFormatError(int lasterror) {
    wchar_t *s = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
               NULL, lasterror,
               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
               (LPWSTR)&s, 0, NULL);
    TIRCriticalError(s);
}


// All-Project globals
wchar_t wcs_current_user[MAX_USERNAME];

int main(int argc, char **argv) {
    sck_init();
    mm_scrint();
    while (1) {
        int ch;
        switch (ch = mm_kbdin()) {
            case 0:
            case -1:
                break;
            case 27:
                exit(0);
                break;
            case '\r':
                msg_sendtext(L"Hello world!");
                break;
            default:
                printf("%lc", (wchar_t)ch);
                break;
        }

        message_t *recv = msg_recv();
        if (recv != NULL)
            mm_scroll(recv);
        
        mm_scrflush();
    }
    return 0;
}