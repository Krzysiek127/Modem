#include "message.h"

extern wchar_t wcs_current_user[MAX_USERNAME];
extern wchar_t wcs_toastbuf[MAX_TOAST];

static uint32_t u32_thread = 0;

static DWORD written;

/* Forward declarations for static functions */
static message_t *msg_filter(message_t **msg);

void sck_sendmsg(message_t *msg) {
    msg->u32_checksum = crc32(msg, sizeof(message_t) - sizeof(uint32_t));   // Exclude chksum itself

    if (send(*sck_getmainsock(), (const char*)msg, sizeof(message_t), 0) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        switch (err) {
            case WSAEWOULDBLOCK:
                Sleep(TCP_SLEEP);
                return;
            default:
                TIRCFormatError(err);
        }
    }
}

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

message_t *msg_setth(message_t **msgptr, uint32_t thread) {
    (*msgptr)->u32_thread = thread;
    return *msgptr;
}

message_t *msg_setflag(message_t **msgptr, uint8_t flags) {
    (*msgptr)->uc_flags = flags;
    return *msgptr;
}

message_t *msg_addr(message_t **msgptr, wchar_t *addr) {
    wmemcpy((*msgptr)->wcs_address, addr, __min(wcslen(addr), MAX_USERNAME));
    return *msgptr;
}
// Just a convention
void msg_free(message_t *msg) {
    if (msg != NULL)
        free(msg);
}


/* High level functions */
message_t *msg_sendtext(wchar_t *message, wchar_t *address) {
    message_t *msg = msg_create();
    msg_type(&msg, MTYPE_TEXT);
    msg_body(&msg, message);

    if (address != NULL)
        msg_addr(&msg, address);
    
    msg_setth(&msg, u32_thread);

    sck_sendmsg(msg);
    
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
        return NULL;
    }

    // For some reason it error here with
    // "an established connection was aborted by the software in your host machine"
    TIRCFormatError(err);
}

void msg_sendfile(wchar_t *path) {
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

    uint32_t byteSize = GetFileSize(hFile, NULL),
             chunkSz  = byteSize / (MAX_BODY * sizeof(wchar_t)),
             remSize  = byteSize % (MAX_BODY * sizeof(wchar_t));


    /* Starting message */
    message_t *begin = msg_create();
    msg_type(&begin, MTYPE_DATA_BEGIN);
    msg_body(&begin, (wchar_t*)PathFindFileNameW(path));
    sck_sendmsg(begin);
    msg_free(begin);

    DWORD bytesRead;
    for (uint32_t ch = 0; ch < chunkSz; ch++) {
        message_t *data = msg_create();
        msg_type(&data, MTYPE_DATA);
        if (!ReadFile(hFile, data->wcs_body, MAX_BODY * sizeof(wchar_t), &bytesRead, NULL)) {   // ReadFile reads directly to message buffer
            msg_type(&data, MTYPE_DATA_ERROR);
        }
        
        sck_sendmsg(data);
        msg_free(data);
    }

    /* Ending message */
    message_t *end = msg_create();
    msg_type(&end, MTYPE_DATA_END);
    msg_uarg(&end, remSize);

    if (!ReadFile(hFile, end->wcs_body, remSize, &bytesRead, NULL)) {
        msg_type(&end, MTYPE_DATA_ERROR);
    }
    
    sck_sendmsg(end);
    msg_free(end);

    CloseHandle(hFile);
}
uint32_t get_current_thread(void) {
    return u32_thread;
}
void set_current_thread(uint32_t th) {
    u32_thread = th;
}

static message_t *msg_filter(message_t **msg) {
    wchar_t *wcs_addr = (*msg)->wcs_address;

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
    if ((*msg)->uc_flags & MFLAG_BROADCAST)
        goto skip_thread_check;
    
    // Nor are we in the same thread
    if ((*msg)->u32_thread != u32_thread) {
        msg_free(*msg);
        return NULL;
    }

skip_thread_check:
    // PING flag
    if ((*msg)->uc_flags & MFLAG_PING) {
        MessageBeep(MB_ICONASTERISK);
        msg_free(*msg);
        return NULL;
    }
    static HANDLE hFileRecv;    /* Without static, the function would override the object because it's called many many times */
    switch((*msg)->uc_type) {
        case MTYPE_TEXT:
            return *((*msg)->wcs_body) ? *msg : NULL;       // Beautiful; if the first byte of body is NULL then we return NULL or the message otherwise
        case MTYPE_DATA_BEGIN:
            // No if (shouldDownload) because if someone wants private send then just go into different thread
            if (!wcscmp(wcs_addr, wcs_current_user)) // This prevents the file being downloaded to the user sending it
                break;
            
            hFileRecv = CreateFileW(
                    (*msg)->wcs_body,       // Filename is sent in the message body  
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
        
        case MTYPE_DATA:
            if (hFileRecv != INVALID_HANDLE_VALUE)
                WriteFile(hFileRecv, (*msg)->wcs_body, MAX_BODY * sizeof(wchar_t), &written, NULL);
            break;
        case MTYPE_DATA_END:
            WriteFile(hFileRecv, (*msg)->wcs_body, (*msg)->u32_argument, &written, NULL);   // Write remaining bytes
            CloseHandle(hFileRecv);
            break;
        case MTYPE_DATA_ERROR:
            MessageBoxW(NULL, L"File transfer error", L"Transfer error", MB_ICONEXCLAMATION);
            CloseHandle(hFileRecv);
            break;
        
        case MTYPE_CONNECT:
            mm_toast(L"%ls joined!", (*msg)->wcs_username);
            break;
        case MTYPE_DXCONNECT:
            mm_toast(L"%ls left!", (*msg)->wcs_username);
            break;
        
        case MTYPE_SHUTDOWN:
            MessageBoxW_Format(L"You've been kicked from the server by %ls", (*msg)->wcs_username);
            exit(0);
            break;
    }
    msg_free(*msg);
    return NULL;
}