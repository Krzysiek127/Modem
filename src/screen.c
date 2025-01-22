#include "screen.h"

#define HEADER_SIZE (MAX_BODY + MAX_TOAST + 6)

// dummy windows variables
static DWORD written;
CONSOLE_SCREEN_BUFFER_INFO csbi;

extern wchar_t wcs_currentUser[MAX_USERNAME];

HANDLE hIn  = NULL;  // stdin
HANDLE hOut = NULL;  // stdout

// screen buffer
static struct screenBuffer{
    wchar_t toastBuffer[MAX_TOAST];

    message_t *msgBuffer[MSG_BUFFER_SIZE];

    wchar_t lineBuffer[MAX_BODY];
    uint8_t lineBufferIndex;
} screen;

// console flags to restore on exit
DWORD savedFlags;


/* Forward declaration for static functions */
static void mm_msgPrint(message_t *msg);
static void mm_clearLineBuffer(void);

// gets char from input stream
static wchar_t mm_kbdIn(void);

static inline void mm_cursorVis(bool state) {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = state;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

/* global declarations */

inline void mm_toast(const wchar_t *text) {
    swprintf(screen.toastBuffer, MAX_TOAST, L"%ls", text);
    mm_clearScreen();
}

void mm_toastf(const wchar_t *format, ...) {
    va_list argptr;

    va_start(argptr, format);
    vswprintf(screen.toastBuffer, MAX_TOAST, format, argptr);
    va_end(argptr);

    mm_clearScreen();
}

void mm_clearMsgBuffer(void) {
    for (int i = 0; i < MSG_BUFFER_SIZE; i++) {
        if (screen.msgBuffer[i] != NULL)
            msg_free(screen.msgBuffer[i]);
    }
}

void mm_kbdLine(void) {
    const wchar_t ch = mm_kbdIn();

    switch (ch) {
        // Invalid chars
        case 0: break;
        
        case 27: { // ESC
            message_t *dxconn = msg_create(MSG_DISCONNECT, BROADCAST_THREAD);

            msg_send(dxconn);
            msg_free(dxconn);

            safeExit();
            break;
        }    
        case L'\r': { // Enter

            if (screen.lineBufferIndex == 0)    // Dont send empty message
                break;

            // private message
            wchar_t *priv = NULL;

            uint8_t msgOffset = 0;
            uint32_t msgThread = getCurrentThread();

            // commands / special messages
            if (screen.lineBuffer[0] == L'/') {

                // File transfer (/send)
                if (!wcsncmp(screen.lineBuffer, L"/send", 5)) {
                    wchar_t *sfn = OpenFileDialog();
            
                    msg_sendFile(sfn);
                    free(sfn);                    

                    mm_clearLineBuffer();
                    break;
                }
                
                // Change thread (/thread+<num>)
                if (!wcsncmp(screen.lineBuffer, L"/thread+", 8)) {
                    const uint32_t th = wcstou32(screen.lineBuffer + 8);

                    if (th >= BROADCAST_THREAD) {
                        mm_toast(L"Thread number too high.");
                        mm_clearLineBuffer();
                        return;
                    }
                    
                    setCurrentThread((uint16_t)th);
                    mm_toastf(L"Connected to thread %u.", getCurrentThread());
                    
                    mm_clearLineBuffer();
                    return;
                }

                // quit the app
                if (!wcsncmp(screen.lineBuffer, L"/quit", 5)) {
                    message_t *dxconn = msg_create(MSG_DISCONNECT, BROADCAST_THREAD);

                    msg_send(dxconn);
                    msg_free(dxconn);

                    safeExit();
                    return;
                }

                // Private message (/priv+<user>)
                if (!wcsncmp(screen.lineBuffer, L"/priv+", 6)) {
                    msgOffset = wcs_scan(screen.lineBuffer + 6);
                    priv = memcpy_n(screen.lineBuffer + 6, MAX_USERNAME * sizeof(wchar_t));
                    
                    msgOffset += 6; // remove starting "/priv+"
                }

                // broadcast the message (/broad)
                if (!wcsncmp(screen.lineBuffer, L"/broad", 6)) {
                    msgOffset = 6; // remove starting "/broad"
                    msgThread = BROADCAST_THREAD;
                }
            }

            message_t *sdmsg = msg_create(MSG_TEXT, msgThread);
            msg_setContent(sdmsg, priv, screen.lineBuffer + msgOffset);
            if (priv != NULL)
                free(priv);

            msg_send(sdmsg);

            // Add the message to queue
            mm_scroll(sdmsg);
            mm_clearLineBuffer();
            break;
        }
        case 127:   // Backspace
            if (screen.lineBufferIndex)
                screen.lineBuffer[--screen.lineBufferIndex] = 0;
            mm_clearScreen();
            break;
        case L' ': // dont prepend spaces in messages
            if (screen.lineBufferIndex == 0)
                break;
        default:
            if (screen.lineBufferIndex < MAX_BODY && iswprint(ch))
                screen.lineBuffer[screen.lineBufferIndex++] = ch;
            break;
    }
}


void mm_printHeader(void) {
    SetConsoleCursorPosition(hOut, (COORD) {0, 0});

    wchar_t header[MAX_TOAST + MAX_BODY + 6] = {0};

    swprintf(
        header, MAX_TOAST + MAX_BODY + 6,
        L"%ls@%hu> %ls\n%ls",
        wcs_currentUser, getCurrentThread(), screen.lineBuffer, screen.toastBuffer
    );

    WriteConsoleW(hOut, header, HEADER_SIZE, &written, NULL);

    /*
    // 3 io calls just for 2 lines..
    wprintf(L"%ls@%hu> ", wcs_currentUser, getCurrentThread());
    WriteConsoleW(hOut, screen.lineBuffer, MAX_BODY, &written, NULL);

    SetConsoleCursorPosition(hOut, (COORD) {0, 1});
    WriteConsoleW(hOut, screen.toastBuffer, MAX_TOAST, &written, NULL);
    */
}


/* Normal functions */

void mm_screenInit(void) {
    memset(&screen, 0, sizeof(screen));

    hIn  = GetStdHandle(STD_INPUT_HANDLE);
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    TIRCAssert(hIn == NULL, L"Failed to initialize STDIN handle!");
    TIRCAssert(hOut == NULL, L"Failed to initialize STDOUT handle!");

    //SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");

    GetConsoleMode(hIn, &savedFlags);
    
    SetConsoleMode(
        hIn,
        ((~ENABLE_LINE_INPUT) & ~ENABLE_ECHO_INPUT) | ENABLE_PROCESSED_INPUT
    );

    GetConsoleScreenBufferInfo(hOut, &csbi);
    mm_cursorVis(false);
}


void mm_clearScreen(void) {
    FillConsoleOutputCharacterW(
        hOut, 
        ' ', 
        (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y, 
        (COORD){0, 0}, 
        &written
    );

    FillConsoleOutputAttribute(
        hOut, 
        csbi.wAttributes, 
        (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y, 
        (COORD){0, 0}, 
        &written
    );

    SetConsoleCursorPosition(hOut, (COORD){0, 0});
}

void mm_screenFlush(void) {
    for (size_t i = 0; i < MSG_BUFFER_SIZE; i++) {
        SetConsoleCursorPosition(hOut, (COORD){0, START_LINE + (i * 2)});
        mm_msgPrint(screen.msgBuffer[i]);
    }
}


void mm_scroll(message_t *msg) {
    msg_free(screen.msgBuffer[0]);
    for (size_t i = 0; i < MSG_BUFFER_SIZE; i++) {    // Add display limit depending on term resolution
        screen.msgBuffer[i] = screen.msgBuffer[i + 1];
    }
    screen.msgBuffer[MSG_BUFFER_SIZE - 1] = msg;
}

/* Static functions */

static wchar_t mm_kbdIn(void) {
    INPUT_RECORD IR;
    DWORD EVENTSREAD;

    if (ReadConsoleInputW(hIn, &IR, 1, &EVENTSREAD))
        return (IR.EventType == KEY_EVENT && IR.Event.KeyEvent.bKeyDown) ? IR.Event.KeyEvent.uChar.UnicodeChar : 0;

    return 0;
}

static void mm_msgPrint(message_t *msg) {
    if (msg == NULL)
        return;

    if (*(msg->contents.wcs_address)) {
        SetConsoleTextAttribute(hOut, FOREGROUND_RED);
        WriteConsoleW(hOut, L"PRIV ", 6, &written, NULL);
    }

    SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN);

    if (msg->thread != BROADCAST_THREAD) {
        wchar_t thread[11] = {0};
        swprintf(thread, 11, L"%hu+", msg->thread);
        WriteConsoleW(hOut, thread, wcslen(thread), &written, NULL);
    }
    else
        WriteConsoleW(hOut, L"BROADCAST+", 10, &written, NULL);

    SetConsoleTextAttribute(hOut, FOREGROUND_GREEN);
    WriteConsoleW(hOut, msg->contents.wcs_username, wcslen(msg->contents.wcs_username), &written, NULL);


    SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY);
    WriteConsoleW(hOut, L"\t> ", 3, &written, NULL);


    SetConsoleTextAttribute(hOut, FOREGROUND_DEFAULT);
   
    WORD wTextAttr = 0;

    for (wchar_t *wch = msg->contents.wcs_body; *wch; ++wch) {
        if(*wch != L'$') {
            WriteConsoleW(hOut, wch, 1, &written, NULL);
            continue;
        }

        ++wch;
        switch (*wch) {
            case L'$':  // print '$' char itself
                WriteConsoleW(hOut, wch, 1, &written, NULL);
                break;
            
            /* Intensities */
            case L'>': wTextAttr |= FOREGROUND_INTENSITY; break;
            case L'<': wTextAttr |= BACKGROUND_INTENSITY; break;      

            /* Foregrounds */
            case L'r': wTextAttr |= FOREGROUND_RED;     break;
            case L'g': wTextAttr |= FOREGROUND_GREEN;   break;
            case L'b': wTextAttr |= FOREGROUND_BLUE;    break;

            /* Backgrounds */
            case L'R': wTextAttr |= BACKGROUND_RED;     break;
            case L'G': wTextAttr |= BACKGROUND_GREEN;   break;
            case L'B': wTextAttr |= BACKGROUND_BLUE;    break;

            case L'!':
                SetConsoleTextAttribute(hOut, FOREGROUND_DEFAULT);
                wTextAttr = 0;
                continue;

            default: WriteConsoleW(hOut, wch, 1, &written, NULL); break;
        }

        SetConsoleTextAttribute(hOut, wTextAttr == 0 ? FOREGROUND_DEFAULT : wTextAttr);
    }
    
    SetConsoleTextAttribute(hOut, FOREGROUND_DEFAULT);
    return;
}

static void mm_clearLineBuffer(void) {
    wmemset(screen.lineBuffer, 0, MAX_BODY);
    screen.lineBufferIndex = 0;
    mm_clearScreen();
}

// screen cleanup, sets previous console state
void mm_cleanup(void) {
    if (hIn && hOut) { // dont cleanup screen stuff if it wasnt even initialized
        mm_clearScreen();
        mm_clearMsgBuffer();
        mm_cursorVis(true);
        SetConsoleMode(hIn, savedFlags);
        memset(&screen, 0, sizeof(screen));
    }
}