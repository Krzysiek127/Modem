#include "screen.h"


#define START_LINE 3

HANDLE hInput, hOutput;
CONSOLE_SCREEN_BUFFER_INFO csbi;

wchar_t wcs_toastbuf[MAX_TOAST];

extern wchar_t wcs_current_user[MAX_USERNAME];

#define VECTOR_LENGTH   10
static message_t *msg_vector[VECTOR_LENGTH];

static wchar_t wcs_linebuf[MAX_BODY] = {0};
static size_t lbuf_index = 0;

static DWORD written;

/* Forward declaration for static functions */
static void mm_msgFormat(message_t *msg);
static void mm_clearBuffer(void);


void mm_toast(const wchar_t *format, ...) {
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

void mm_cursorVis(bool state) { // Cursor visiblity not Kurvinox XDDD
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOutput, &cursorInfo);
    cursorInfo.bVisible = state;
    SetConsoleCursorInfo(hOutput, &cursorInfo);
}

void mm_kbdLine(void) {
    const int ch = mm_kbdIn();

    switch (ch) {
        case 0:     // Invalid chars
        case -1: break;
        case 27:    // ESC
            message_t *dxconn = msg_create();
            msg_type(&dxconn, MSG_DXCONNECT);
            msg_setFlags(&dxconn, MFLAG_BROADCAST);
            msg_setThread(&dxconn, getCurrentThread());

            msg_send(dxconn);

            mm_cursorVis(true);
            exit(0);
            break;

        case '\r':  // Enter
            if (wcs_linebuf[0] == 0)    // Dont send empty message
                break;

            // File transfer
            if (!wcscmp(wcs_linebuf, L"/send")) {
                wchar_t *sfn = OpenFileDialog();
        
                msg_sendFile( sfn );
                free(sfn);
                goto clear_then_exit_switch;
            }
            
            // Change thread
            if (!wcsncmp(wcs_linebuf, L"/thread+", 8)) {
                setCurrentThread( wcstou32(wcs_linebuf + 8) );
                mm_toast(L"Connected to thread %u", getCurrentThread());
                
                goto clear_then_exit_switch;
            }

            // Send to private user
            wchar_t *priv = NULL;
            size_t privOffset = 0;

            if (!wcsncmp(wcs_linebuf, L"/priv+", 6)) {
                privOffset = wcs_scan(wcs_linebuf + 6);
                priv = wcs_copy_n(wcs_linebuf + 6, privOffset - 1);

                privOffset += 6;
            }

            message_t *sdmsg = msg_sendText(wcs_linebuf + privOffset, priv);    // Create message

            // If line CONTAINS $PING then set the flag
            if (!wcsstr(wcs_linebuf, L"!PING") != 0) {
                msg_setFlags(&sdmsg, sdmsg->uc_flags | MFLAG_PING);
            }

            // If line starts with /broad then broadcast the message
            if (!wcsncmp(wcs_linebuf, L"/broad", 6)) {
                msg_setFlags(&sdmsg, sdmsg->uc_flags | MFLAG_BROADCAST);
            }

            // Add the message to queue
            mm_scroll(sdmsg);

clear_then_exit_switch:
            mm_clearBuffer();
            break;

        case 127:   // Backspace
            if (lbuf_index)
                wcs_linebuf[--lbuf_index] = 0;
            mm_screenClear();
            break;

        default:
            if (lbuf_index < MAX_BODY && isprint(ch))
                wcs_linebuf[lbuf_index++] = (wchar_t)ch;
            break;
    }
}
void mm_printLbuf(void) {
    SetConsoleCursorPosition(hOutput, (COORD) {0, 0});
    wprintf(L"%ls@%u> ", wcs_current_user, getCurrentThread());
    WriteConsoleW(hOutput, wcs_linebuf, lbuf_index, &written, NULL);

    SetConsoleCursorPosition(hOutput, (COORD) {0, 1});
    WriteConsoleW(hOutput, wcs_toastbuf, MAX_TOAST, &written, NULL);
}


/* Normal functions */

void mm_screenInit(void) {
    hInput = GetStdHandle(STD_INPUT_HANDLE);
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
}

void mm_screenClear(void) {
    FillConsoleOutputCharacter(hOutput, ' ', csbi.dwSize.X * csbi.dwSize.Y, (COORD){0, 0}, &written);
    FillConsoleOutputAttribute(hOutput, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, (COORD){0, 0}, &written);

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

int mm_kbdIn(void) {
    TIRCAssert(hInput == NULL, L"Uninitialized STDIN handle!");
    INPUT_RECORD IR;
    DWORD EVENTSREAD;

    if (PeekConsoleInputW(hInput, &IR, 1, &EVENTSREAD) && EVENTSREAD) {
        ReadConsoleInputW(hInput, &IR, 1, &EVENTSREAD);
        return (IR.EventType == KEY_EVENT && IR.Event.KeyEvent.bKeyDown) ? IR.Event.KeyEvent.uChar.UnicodeChar : -1;
    }
    return -1;
}

/* Static functions */

static void mm_msgFormat(message_t *msg) {
    if (msg == NULL)
        return;

    if (*(msg->contents.wcs_address)) {
        SetConsoleTextAttribute(hOutput, FOREGROUND_RED);
        WriteConsoleW( hOutput, L"PRIV ", 6, &written, NULL );
    }
    SetConsoleTextAttribute(hOutput, FOREGROUND_DEFAULT);

    /* For now okay??? */
    printf("%u+", msg->u32_thread);

    SetConsoleTextAttribute(hOutput, FOREGROUND_GREEN);
    WriteConsoleW(hOutput, msg->contents.wcs_username, MAX_USERNAME, &written, NULL);   // I'm not sure MAX_USERNAME is correct here


    SetConsoleTextAttribute(hOutput, FOREGROUND_INTENSITY);
    WriteConsoleW(hOutput, L"> ", 3, &written, NULL);

    SetConsoleTextAttribute(hOutput, FOREGROUND_DEFAULT);

   
    bool shouldRender = true; // for now its useless
    WORD wTextAttr = 0;
    wchar_t *privUser, *privUserPers;

    for (wchar_t *wch = msg->contents.wcs_body; *wch; wch++) {
        switch (*wch) {
            case L'$':
                ++wch;
                switch (*wch) {
                    case L'$':
                        goto print;

                    case L'!':
                        SetConsoleTextAttribute(hOutput, FOREGROUND_DEFAULT);
                        wTextAttr = 0;
                        shouldRender = true;
                        continue;

                    case L'>':
                        wTextAttr |= FOREGROUND_INTENSITY;
                        break;

                    case L'<':
                        wTextAttr |= BACKGROUND_INTENSITY;
                        break;
                    

                    /* Foregrounds */
                    case L'r':
                        wTextAttr |= FOREGROUND_RED;
                        break;

                    case L'g':
                        wTextAttr |= FOREGROUND_GREEN;
                        break;

                    case L'b':
                        wTextAttr |= FOREGROUND_BLUE;
                        break;


                    /* Backgrounds */
                    case L'R':
                        wTextAttr |= BACKGROUND_RED;
                        break;

                    case L'G':
                        wTextAttr |= BACKGROUND_GREEN;
                        break;

                    case L'B':
                        wTextAttr |= BACKGROUND_BLUE;
                        break;

                    default:
                        goto print;
                }
                SetConsoleTextAttribute(hOutput, wTextAttr == 0 ? FOREGROUND_DEFAULT : wTextAttr);
                break;

            default:
            print:
                if (shouldRender)
                    WriteConsoleW(hOutput, wch, 1, &written, NULL);
                break;
        }
    }
    
    SetConsoleTextAttribute(hOutput, FOREGROUND_DEFAULT);
    return;
}

static void mm_clearBuffer(void) {
    wmemset(wcs_linebuf, 0, MAX_BODY);
    lbuf_index = 0;
    mm_screenClear();
}