#include "message.h"

extern wchar_t wcs_current_user[MAX_USERNAME];
extern wchar_t wcs_toastbuf[MAX_TOAST];

static uint32_t u32_thread = 0;

static DWORD written;

/* Forward declarations for static functions */
static message_t *msg_filter(message_t **msg);


void msg_send(message_t *msg) {
    msg->u32_checksum = crc32(msg, sizeof(message_t) - sizeof(uint32_t));   // Exclude chksum itself

    sock_send(msg, sizeof(message_t));
}

// Returns NULL when no message was recv or shouldn't be displayed
message_t *msg_recieve(void) {
    message_t *rcvmsg = calloc(1, sizeof(message_t)); // Idiot forgor to allocate memry

    if (sock_recieve(rcvmsg, sizeof(message_t)))
        return msg_filter(&rcvmsg);
    
    /* Something happened */
    msg_free(rcvmsg);
    int err = WSAGetLastError();
    if (err == WSAEWOULDBLOCK) {
        Sleep(TCP_SLEEP);
        return NULL;
    }

    TIRCFormatError(err);

    // compilers sometimes see that a function in some cases does not return
    // a value even when in those cases program terminates..
    // so here it is.
    return NULL;
}



message_t *msg_create(void) {
    message_t *tmp = calloc(1, sizeof(message_t));
    tmp->mmver = MMVER;
    tmp->tm_timestamp = time(NULL);
    wmemcpy(tmp->contents.wcs_username, wcs_current_user, MAX_USERNAME);
    return tmp;
}

message_t *msg_type(message_t **msgptr, uint8_t type) {
    (*msgptr)->uc_type = type;
    return *msgptr;
}

message_t *msg_body(message_t **msgptr, wchar_t *body) {
    wmemcpy((*msgptr)->contents.wcs_body, body, __min(wcslen(body), MAX_BODY));
    return *msgptr;
}

message_t *msg_uarg(message_t **msgptr, uint32_t arg) {
    (*msgptr)->u32_argument = arg;
    return *msgptr;
}

message_t *msg_setThread(message_t **msgptr, uint32_t thread) {
    (*msgptr)->u32_thread = thread;
    return *msgptr;
}

message_t *msg_setFlags(message_t **msgptr, uint8_t flags) {
    (*msgptr)->uc_flags = flags;
    return *msgptr;
}

message_t *msg_addr(message_t **msgptr, wchar_t *addr) {
    wmemcpy((*msgptr)->contents.wcs_address, addr, __min(wcslen(addr), MAX_USERNAME));
    return *msgptr;
}

// Just a convention
void msg_free(message_t *msg) {
    if (msg != NULL) {
        memset(msg, 0, sizeof(message_t));
        free(msg);
        msg = NULL;
    }
}


message_t *msg_create2(
    const msgType_t type,
    const uint32_t thread,
    const uint32_t arg,
    const uint8_t flags
) {
    message_t *msg = calloc(1, sizeof(message_t));

    msg->mmver = MMVER;
    msg->tm_timestamp = time(NULL);

    msg->uc_type = type;
    msg->u32_thread = thread;
    msg->u32_argument = arg;
    msg->uc_flags = flags;

    return msg;
}

void msg_setContent(
    message_t *restrict msg,
    const wchar_t *addr,
    const wchar_t *body
) {
    memcpy(msg->contents.wcs_username, wcs_current_user, MAX_USERNAME * sizeof(wchar_t));
    memcpy(msg->contents.wcs_address, addr, MAX_USERNAME * sizeof(wchar_t));
    memcpy(msg->contents.wcs_body, body, MAX_BODY * sizeof(wchar_t));
}


/* High level functions */
message_t *msg_sendText(wchar_t *text, wchar_t *addr) {
    message_t *msg = msg_create();
    msg_type(&msg, MSG_TEXT);
    msg_body(&msg, text);

    if (addr != NULL)
        msg_addr(&msg, addr);
    
    msg_setThread(&msg, u32_thread);

    msg_send(msg);
    
    return msg;
}


void msg_sendFile(wchar_t *path) {
    HANDLE hFile = CreateFileW(
        path,                
        GENERIC_READ,            
        FILE_SHARE_READ,         
        NULL,                    
        OPEN_EXISTING,           
        FILE_ATTRIBUTE_NORMAL,   
        NULL                     
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxW(NULL, L"Could not open the file!", L"File error", MB_ICONASTERISK);
        return;
    }

    const uint32_t byteSize = GetFileSize(hFile, NULL);

    uint32_t chunkSz  = byteSize / (MAX_BODY * sizeof(wchar_t));
    uint32_t remSize  = byteSize % (MAX_BODY * sizeof(wchar_t));


    /* Starting message */
    message_t *begin = msg_create();
    msg_type(&begin, MSG_DATA_BEGIN);
    msg_body(&begin, PathFindFileNameW(path));
    msg_send(begin);
    msg_free(begin);

    DWORD bytesRead;
    for (uint32_t ch = 0; ch < chunkSz; ch++) {
        message_t *data = msg_create();
        msg_type(&data, MSG_DATA);

        // ReadFile reads directly to message buffer
        if (!ReadFile(hFile, data->contents.wcs_body, sizeof(data->contents.wcs_body), &bytesRead, NULL))
            msg_type(&data, MSG_DATA_ERROR);
        
        msg_send(data);
        msg_free(data);
    }

    /* Ending message */
    message_t *end = msg_create();
    msg_type(&end, MSG_DATA_END);
    msg_uarg(&end, remSize);

    if (!ReadFile(hFile, end->contents.wcs_body, remSize, &bytesRead, NULL))
        msg_type(&end, MSG_DATA_ERROR);
    
    msg_send(end);
    msg_free(end);

    CloseHandle(hFile);
}


uint32_t getCurrentThread(void) { return u32_thread; }
void setCurrentThread(const uint32_t th) { u32_thread = th; }


static message_t *msg_filter(message_t **msg) {
    const wchar_t *wcs_addr = (*msg)->contents.wcs_address;

    if ((*msg)->mmver != MMVER) {
        mm_toast(L"Message with invalid protocol version received");
        msg_free(*msg);
        return NULL;
    }

    // CRC32 check
    if (crc32(*msg, sizeof(message_t) - sizeof(uint32_t)) != (*msg)->u32_checksum) {
        mm_toast(L"Message with invalid CRC32 received!");
        msg_free(*msg);
        return NULL;
    }

    // The message is not addressed to us...
    if (wcs_addr[0] != L'\0' && wcscmp(wcs_addr, wcs_current_user) != 0) {
        msg_free(*msg);
        return NULL;
    }   
    
    // Check if message has a flag telling it not to check thread TODO: Replace goto with if
    if (!((*msg)->uc_flags & MFLAG_BROADCAST)) {

        // Nor are we in the same thread
        if ((*msg)->u32_thread != u32_thread) {
            msg_free(*msg);
            return NULL;
        }
    }

    // PING flag
    if ((*msg)->uc_flags & MFLAG_PING) {
        MessageBeep(MB_ICONASTERISK);
        msg_free(*msg);
        return NULL;
    }

    static HANDLE hFileRecv;    /* Without static, the function would override the object because it's called many many times */
    switch((*msg)->uc_type) {
        case MSG_TEXT:
            return *((*msg)->contents.wcs_body) ? *msg : NULL;       // Beautiful; if the first byte of body is NULL then we return NULL or the message otherwise

        case MSG_DATA_BEGIN:
            // No if (shouldDownload) because if someone wants private send then just go into different thread
            if (!wcscmp(wcs_addr, wcs_current_user)) // This prevents the file being downloaded to the user sending it
                break;
            
            hFileRecv = CreateFileW(
                (*msg)->contents.wcs_body,       // Filename is sent in the message body  
                GENERIC_WRITE,        
                0,                    
                NULL,                 
                CREATE_ALWAYS,        
                FILE_ATTRIBUTE_NORMAL,
                NULL                  
            );

            if (hFileRecv == INVALID_HANDLE_VALUE)
                mm_toast(L"The file cannot be downloaded");
            break;
        
        case MSG_DATA:
            if (hFileRecv != INVALID_HANDLE_VALUE)
                WriteFile(hFileRecv, (*msg)->contents.wcs_body, MAX_BODY * sizeof(wchar_t), &written, NULL);
            break;

        case MSG_DATA_END:
            WriteFile(hFileRecv, (*msg)->contents.wcs_body, (*msg)->u32_argument, &written, NULL);   // Write remaining bytes
            CloseHandle(hFileRecv);
            break;

        case MSG_DATA_ERROR:
            MessageBoxW(NULL, L"File transfer error", L"Transfer error", MB_ICONEXCLAMATION);
            CloseHandle(hFileRecv);
            break;
        
        case MSG_CONNECT:
            mm_toast(L"%ls joined!", (*msg)->contents.wcs_username);
            break;

        case MSG_DXCONNECT:
            mm_toast(L"%ls left!", (*msg)->contents.wcs_username);
            break;
        
        case MSG_SHUTDOWN:
            MessageBoxW_Format(L"You've been kicked from the server by %ls", (*msg)->contents.wcs_username);
            exit(0);
            break;
    }

    msg_free(*msg);
    return NULL;
}