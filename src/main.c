#include "screen.h"
#include "message.h"

// All-Project globals
wchar_t wcs_current_user[MAX_USERNAME];

int main(int argc, char **argv) {
    wcscpy(wcs_current_user, L"Krzysiek");

    sck_init();
    mm_scrint();
    mm_clearscr();

    message_t *conn = msg_create();
    msg_type(&conn, MTYPE_CONNECT);
    msg_setflag(&conn, MFLAG_BROADCAST);
    msg_setth(&conn, get_current_thread());
    send(*sck_getmainsock(), (const char*)conn, sizeof(message_t), 0);

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