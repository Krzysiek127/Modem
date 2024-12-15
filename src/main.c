#include "screen.h"

// All-Project globals
wchar_t wcs_current_user[MAX_USERNAME];

int main(int argc, char **argv) {
    wcscpy(wcs_current_user, L"Krzysiek");

    sck_init();
    mm_scrint();
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