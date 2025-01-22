#include "message.h"

extern wchar_t wcs_currentUser[MAX_USERNAME];

static uint16_t currentThread = 0;

static DWORD written;

static void msg_dump(const char *restrict file, const message_t *msg);

void msg_send(message_t *msg) {
    msg->timestamp = time(NULL);
    msg->u32_checksum = crc32(msg, sizeof(message_t) - sizeof(uint32_t));   // Exclude checksum itself

    msg_dump("sended.txt", msg);
    sock_send(msg, sizeof(message_t));
}

message_t *msg_recieve(void) {
    message_t *rcvmsg = calloc(1, sizeof(message_t));
    sock_recieve((void **)&rcvmsg, sizeof(message_t));
    if (rcvmsg != NULL)
        msg_dump("recieved.txt", rcvmsg);
    
    return rcvmsg;
}


void msg_free(message_t *msg) {
    if (msg != NULL) {
        memset(msg, 0, sizeof(message_t));
        free(msg);
    }
}

message_t *msg_init(void) {
    message_t *msg = calloc(1, sizeof(message_t));
    msg->mmver = MMVER;
    wmemcpy(msg->contents.wcs_username, wcs_currentUser, MAX_USERNAME);

    return msg;
}

message_t *msg_create(const msgType_t type, const uint16_t thread) {
    message_t *msg = msg_init();

    msg->type = type;
    msg->thread = thread;

    return msg;
}

void msg_setContent(message_t *restrict msg, const wchar_t *addr, const wchar_t *body) {
    if (addr != NULL)
        memcpy(msg->contents.wcs_address, addr, MAX_USERNAME * sizeof(wchar_t));
    memcpy(msg->contents.wcs_body, body, MAX_BODY * sizeof(wchar_t));
}


/* High level functions */

void msg_sendFile(const wchar_t *path) {
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

    const uint64_t byteSize = GetFileSize(hFile, NULL);
    const uint32_t chunkSz  = byteSize / (MAX_BODY * sizeof(wchar_t));
    const uint32_t remSize  = byteSize % (MAX_BODY * sizeof(wchar_t));

    /* Starting message */
    message_t *begin = msg_init();
    begin->type = MSG_DATA_BEGIN;

    msg_setContent(begin, NULL, PathFindFileNameW(path));
    msg_send(begin);
    msg_free(begin);

    DWORD bytesRead;

    message_t *data = msg_init();
    data->type = MSG_DATA;
    for (uint32_t ch = 0; ch < chunkSz; ch++) {

        // ReadFile reads directly to message buffer
        if (!ReadFile(hFile, data->contents.wcs_body, sizeof(data->contents.wcs_body), &bytesRead, NULL)) {

            data->type = MSG_DATA_ERROR;
            msg_send(data);
            msg_free(data);
            CloseHandle(hFile);
            return;
        }
        
        msg_send(data);
    }
    msg_free(data);

    /* Ending message */
    message_t *end = msg_init();
    end->type = MSG_DATA_END;

    if (!ReadFile(hFile, end->contents.wcs_body, remSize, &bytesRead, NULL))
        end->type = MSG_DATA_ERROR;
    
    msg_send(end);
    msg_free(end);

    CloseHandle(hFile);
}


uint16_t getCurrentThread(void) { return currentThread; }
void setCurrentThread(const uint16_t th) { currentThread = th; }


bool msg_interpret(message_t *msg) {
    if (msg == NULL)
        return false;

    if (msg->mmver != MMVER) {
        mm_toast(L"Message with invalid protocol version received.");
        msg_free(msg);
        return false;
    }

    // CRC32 check
    if (crc32(msg, sizeof(message_t) - sizeof(uint32_t)) != msg->u32_checksum) {
        mm_toast(L"Message with invalid CRC32 received!");
        msg_free(msg);
        return false;
    }

    // The message is not addressed to us...
    if (msg->contents.wcs_address[0] != L'\0' && wcscmp(msg->contents.wcs_address, wcs_currentUser) != 0) {
        msg_free(msg);
        return false;
    }
    
    // Check if message is broadcasted
    if (msg->thread != BROADCAST_THREAD) {

        // Nor are we in the same thread
        if (msg->thread != currentThread) {
            msg_free(msg);
            return false;
        }
    }

    /* Without static, the function would override the object because it's called many many times */
    static HANDLE hFileRecv;

    switch(msg->type) {
        case MSG_TEXT:
            return *(msg->contents.wcs_body); // Even more beautiful; we interpret the first byte in body as true or false

        case MSG_DATA_BEGIN:
            if (!*msg->contents.wcs_body)
                break;

            if (!wcscmp(msg->contents.wcs_address, wcs_currentUser)) // This prevents the file being downloaded to the user sending it
                break;

            hFileRecv = CreateFileW(
                msg->contents.wcs_body, // Filename is sent in the message body  
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (hFileRecv == INVALID_HANDLE_VALUE)
                mm_toast(L"The file cannot be downloaded.");

            return true;
        
        case MSG_DATA:
            if (hFileRecv != INVALID_HANDLE_VALUE)
                WriteFile(hFileRecv, msg->contents.wcs_body, MAX_BODY * sizeof(wchar_t), &written, NULL);
            break;

        case MSG_DATA_END:
            WriteFile(hFileRecv, msg->contents.wcs_body, MAX_BODY, &written, NULL);   // Write remaining bytes
            CloseHandle(hFileRecv);
            break;

        case MSG_DATA_ERROR:
            mm_toastf(L"User %ls failed to correctly send file.", msg->contents.wcs_username);
            CloseHandle(hFileRecv);
            break;
        
        case MSG_CONNECT:
            mm_toastf(L"%ls joined!", msg->contents.wcs_username);
            break;

        case MSG_DISCONNECT:
            mm_toastf(L"%ls left!", msg->contents.wcs_username);
            break;
        
        case MSG_SHUTDOWN:
            MessageBoxW_Format(L"You've been kicked from the server by %ls", msg->contents.wcs_username);
            safeExit();
            break;
    }

    msg_free(msg);
    return false;
}


static inline void msg_dump(const char *restrict file, const message_t *msg) {
    switch (msg->type) { // messages to not print out
        case MSG_DATA:
        case MSG_DATA_END:
        case MSG_DATA_ERROR: return;
    }
    
    FILE *f = fopen(file, "a");

    fwprintf(f,
        L"/ --- message --- /\n"
        L"mmversion:\t%hu\n"
        L"timestamp:\t%zu\n"
        L"msg_type:\t%hu\n"
        L"thread: \t%hu\n"
        L"contents:\n"
        L"\tuser: %ls\n"
        L"\taddr: %ls\n"
        L"\tbody: %ls\n"
        L"checksum:\t%u\n"
        L"/ --------------- /\n\n",
        msg->mmver, msg->timestamp, msg->type, msg->thread, 
        msg->contents.wcs_username, msg->contents.wcs_address, msg->contents.wcs_body,
        msg->u32_checksum
    );

    fclose(f);
}