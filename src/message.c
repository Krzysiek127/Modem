#include "message.h"

extern wchar_t wcs_current_user[MAX_USERNAME];

static uint32_t u32_thread = 0,
                u32_mask   = 0xFFFFFFFF;
// 0 is default thread to connect and it's P2P (message sent ONLY to this one)

message_t *msg_create(void) {
    message_t *tmp = calloc(1, sizeof(message_t));
    tmp->mmver = MMVER;
    tmp->tm_timestamp = time(NULL);
    wmemcpy(tmp->wcs_username, wcs_current_user, MAX_USERNAME);
    return tmp;
}
message_t *msg_type(message_t **msgptr, uint8_t type) {
    (*msgptr)->uc_type = type;
    return *msgptr;
}

message_t *msg_body(message_t **msgptr, wchar_t *body) {
    wmemcpy((*msgptr)->wcs_body, body, __min(wcslen(body), MAX_BODY));
    return *msgptr;
}

message_t *msg_uarg(message_t **msgptr, uint32_t arg) {
    (*msgptr)->u32_argument = arg;
    return *msgptr;
}

message_t *msg_setth(message_t **msgptr, uint32_t thread, uint32_t threadmask) {
    (*msgptr)->u32_thread = thread;
    (*msgptr)->u32_thmask = threadmask;
    return *msgptr;
}

// Just a convention
void msg_free(message_t *msg) {
    if (msg != NULL)
        free(msg);
}


/* High level functions */
message_t *msg_sendtext(wchar_t *message) {
    message_t *msg = msg_create();
    msg_type(&msg, MTYPE_TEXT);
    msg_body(&msg, message);
    msg_setth(&msg, u32_thread, u32_mask);

    if (send(*sck_getmainsock(), (const char*)msg, sizeof(message_t), 0) == SOCKET_ERROR)
        TIRCFormatError( WSAGetLastError() );
    
    return msg;
}

// Returns NULL when no message was recv or shouldn't be displayed
message_t *msg_recv(void) {
    message_t *rcvmsg = calloc(1, sizeof(message_t)); // Idiot forgor to allocate memry

    int rcv = recv(*sck_getmainsock(), (char *)rcvmsg, sizeof(message_t), 0);
    if (rcv > 0)
        return msg_filter(&rcvmsg);
    
    /* Something happened */
    int err = WSAGetLastError();
    if (err == WSAEWOULDBLOCK) {
        Sleep(TCP_SLEEP);
        return NULL;
    }

    // For some reason it error here with
    // "an established connection was aborted by the software in your host machine"
    TIRCFormatError(err);
}


static message_t *msg_filter(message_t **msg) {
    wchar_t *wcs_addr = (*msg)->wcs_address;

    // The message is not directed to us...
    if (*wcs_addr != 0 && wcscmp(wcs_addr, wcs_current_user)) {
        msg_free(*msg);
        return NULL;
    }

    HANDLE hFileRecv;
    switch((*msg)->uc_type) {
        case MTYPE_TEXT:
            return *msg;
        case MTYPE_DATA_BEGIN:
            // No if (shouldDownload) because if someone wants private send then just go into different thread
            break;
    }   
}