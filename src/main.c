#include "screen.h"

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