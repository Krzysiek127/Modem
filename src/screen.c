#include "screen.h"

HANDLE hInput, hOutput;
CONSOLE_SCREEN_BUFFER_INFO csbi;

wchar_t wcs_toastbuf[MAX_TOAST];

extern wchar_t wcs_current_user[MAX_USERNAME];

#define VECTOR_LENGTH   10
static message_t *msg_vector[VECTOR_LENGTH];

static wchar_t wcs_linebuf[MAX_BODY] = {0};
static size_t lbuf_index = 0;

// disposable dummy windows variable
static DWORD written;

/* Forward declaration for static functions */
static void mm_msgFormat(message_t *msg);
static void mm_clearBuffer(void);

static inline void mm_cursorVis(bool state) {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOutput, &cursorInfo);
    cursorInfo.bVisible = state;
    SetConsoleCursorInfo(hOutput, &cursorInfo);
}


inline void mm_toast(const wchar_t *text) {
    swprintf(wcs_toastbuf, MAX_TOAST, L"%ls", text);
    mm_screenClear();
}

void mm_toastf(const wchar_t *format, ...) {
    va_list argptr;

    va_start(argptr, format);
    vswprintf(wcs_toastbuf, MAX_TOAST, format, argptr);
    va_end(argptr);

    mm_screenClear();
}

void mm_vectorClear(void) {
    for (int i = 0; i < VECTOR_LENGTH; i++) {
        msg_free( msg_vector[i] );
        msg_vector[i] = NULL;
    }
}

// gets char from input stream
static wchar_t mm_kbdIn(void);

void mm_kbdLine(void) {
    const wchar_t ch = mm_kbdIn();

    switch (ch) {
        // Invalid chars
        case 0: break;
        
        case 27: { // ESC
            message_t *dxconn = msg_create(MSG_DISCONNECT, BROADCAST_THREAD);

            msg_send(dxconn);
            msg_free(dxconn);

            mm_cursorVis(true);
            exit(0);
            break;
        }    
        case L'\r': { // Enter

            if (wcs_linebuf[0] == 0)    // Dont send empty message
                break;

            // private message attributes
            wchar_t *priv = NULL;

            uint8_t msgOffset = 0;
            uint32_t msgThread = getCurrentThread();

            // special message
            if (wcs_linebuf[0] == L'/') {

                // File transfer (/send)
                if (!wcsncmp(wcs_linebuf, L"/send", 5)) {
                    wchar_t *sfn = OpenFileDialog();
            
                    msg_sendFile(sfn);
                    free(sfn);                    

                    mm_clearBuffer();
                    break;
                }
                
                // Change thread (/thread+<num>)
                if (!wcsncmp(wcs_linebuf, L"/thread+", 8)) {
                    setCurrentThread(wcstou32(wcs_linebuf + 8));
                    msgThread = getCurrentThread();

                    mm_toastf(L"Connected to thread %u", getCurrentThread());
                    
                    mm_clearBuffer();
                    break;
                }

                // Private message (/priv+<user>)
                if (!wcsncmp(wcs_linebuf, L"/priv+", 6)) {
                    msgOffset = wcs_scan(wcs_linebuf + 6);
                    priv = wcs_copy_n(wcs_linebuf + 6, msgOffset - 1);

                    msgOffset += 6; // remove starting "/priv+"
                }

                // broadcast the message (/broad)
                if (!wcsncmp(wcs_linebuf, L"/broad", 6)) {
                    msgOffset = 6; // remove starting "/broad"
                    msgThread = BROADCAST_THREAD;
                }
            }

            message_t *sdmsg = msg_create(MSG_TEXT, msgThread);
            msg_setContent(sdmsg, priv, wcs_linebuf + msgOffset);
            msg_send(sdmsg);

            // Add the message to queue
            mm_scroll(sdmsg);
            mm_clearBuffer();
            break;
        }
        case 127:   // Backspace
            if (lbuf_index)
                wcs_linebuf[--lbuf_index] = 0;
            mm_screenClear();
            break;

        default:
            if (lbuf_index < MAX_BODY && iswprint(ch))
                wcs_linebuf[lbuf_index++] = ch;
            break;
    }
}


void mm_printLineBuff(void) {
    SetConsoleCursorPosition(hOutput, (COORD) {0, 0});
    wprintf(L"%ls@%u> ", wcs_current_user, getCurrentThread());
    WriteConsoleW(hOutput, wcs_linebuf, lbuf_index, &written, NULL);

    SetConsoleCursorPosition(hOutput, (COORD) {0, 1});
    WriteConsoleW(hOutput, wcs_toastbuf, MAX_TOAST, &written, NULL);
}


/* Normal functions */

void mm_screenInit(void) {
    hInput  = GetStdHandle(STD_INPUT_HANDLE);
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    TIRCAssert(hInput == NULL, L"Failed to initialize STDIN handle!");
    TIRCAssert(hOutput == NULL, L"Failed to initialize STDOUT handle!");

    //SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");
    SetConsoleMode(
        hInput, 
        ((~ENABLE_LINE_INPUT) & ~ENABLE_ECHO_INPUT) | ENABLE_PROCESSED_INPUT
    );

    GetConsoleScreenBufferInfo(hOutput, &csbi);
    mm_cursorVis(false);
}

void mm_screenClear(void) {
    FillConsoleOutputCharacterW(
        hOutput, 
        ' ', 
        (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y, 
        (COORD){0, 0}, 
        &written
    );

    FillConsoleOutputAttribute(
        hOutput, 
        csbi.wAttributes, 
        (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y, 
        (COORD){0, 0}, 
        &written
    );

    //FillConsoleOutputAttribute(hOutput, BACKGROUND_INTENSITY, s.dwSize.X, (COORD){0, 1}, &written);

    SetConsoleCursorPosition(hOutput, (COORD){0, 0});
}

void mm_screenFlush(void) {
    for (size_t i = 0; i < VECTOR_LENGTH; i++) {
        SetConsoleCursorPosition(hOutput, (COORD){0, START_LINE + (i * 2)});
        mm_msgFormat(msg_vector[i]);
    }
}


void mm_scroll(message_t *new) {
    msg_free(msg_vector[0]);
    for (size_t i = 0; i < VECTOR_LENGTH; i++) {    // Add display limit depending on term resolution
        msg_vector[i] = msg_vector[i + 1];
    }
    msg_vector[VECTOR_LENGTH - 1] = new;
}

/* Static functions */

static wchar_t mm_kbdIn(void) {
    TIRCAssert(hInput == NULL, L"Uninitialized STDIN handle!");
    INPUT_RECORD IR;
    DWORD EVENTSREAD;

    if (PeekConsoleInputW(hInput, &IR, 1, &EVENTSREAD) && EVENTSREAD) {
        ReadConsoleInputW(hInput, &IR, 1, &EVENTSREAD);
        return (IR.EventType == KEY_EVENT && IR.Event.KeyEvent.bKeyDown) ? IR.Event.KeyEvent.uChar.UnicodeChar : 0;
    }
    return 0;
}

static void mm_msgFormat(message_t *msg) {
    if (msg == NULL)
        return;

    if (*(msg->contents.wcs_address)) {
        SetConsoleTextAttribute(hOutput, FOREGROUND_RED);
        WriteConsoleW( hOutput, L"PRIV ", 6, &written, NULL);
    }

    SetConsoleTextAttribute(hOutput, FOREGROUND_RED | FOREGROUND_GREEN);

    if (msg->u32_thread != BROADCAST_THREAD)
    {
        wchar_t thread[11] = {0};
        swprintf(thread, 11, L"%u+", msg->u32_thread);
        WriteConsoleW(hOutput, thread, wcslen(thread), &written, NULL);
    }
    else
        WriteConsoleW(hOutput, L"BROADCAST+", 10, &written, NULL);

    SetConsoleTextAttribute(hOutput, FOREGROUND_GREEN);
    WriteConsoleW(hOutput, msg->contents.wcs_username, wcslen(msg->contents.wcs_username), &written, NULL);


    SetConsoleTextAttribute(hOutput, FOREGROUND_INTENSITY);
    WriteConsoleW(hOutput, L"\t> ", 3, &written, NULL);


    SetConsoleTextAttribute(hOutput, FOREGROUND_DEFAULT);
   
    WORD wTextAttr = 0;

    for (wchar_t *wch = msg->contents.wcs_body; *wch; ++wch) {
        if(*wch != L'$') {
            WriteConsoleW(hOutput, wch, 1, &written, NULL);
            continue;
        }

        ++wch;
        switch (*wch) {
            case L'$':  // print '$' char itself
                WriteConsoleW(hOutput, wch, 1, &written, NULL);
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
                SetConsoleTextAttribute(hOutput, FOREGROUND_DEFAULT);
                wTextAttr = 0;
                continue;

            default:
                WriteConsoleW(hOutput, wch, 1, &written, NULL);
                break;
        }

        SetConsoleTextAttribute(hOutput, wTextAttr == 0 ? FOREGROUND_DEFAULT : wTextAttr);
    }
    
    SetConsoleTextAttribute(hOutput, FOREGROUND_DEFAULT);
    return;
}

static void mm_clearBuffer(void) {
    wmemset(wcs_linebuf, 0, MAX_BODY);
    lbuf_index = 0;
    mm_screenClear();
}