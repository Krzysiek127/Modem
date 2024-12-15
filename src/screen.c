#include "screen.h"
#include "dlgbox.h"


#define START_LINE 3

HANDLE hInput, hOutput;
CONSOLE_SCREEN_BUFFER_INFO csbi;

#define VECTOR_LENGTH   10
static message_t *msg_vector[VECTOR_LENGTH];

static wchar_t wcs_linebuf[MAX_BODY];
static size_t lbuf_index = 0;

static DWORD written;

/* Forward declaration for static functions */
static void mm_msgformat(message_t *msg);
static void mm_clearbuf(void);

void mm_kbdline(void) {
    int ch;
    switch (ch = mm_kbdin()) {
        case 0:
        case -1:
            break;
        case 27:
            exit(0);
            break;
        case '\r':
            if (wcs_linebuf[0] == 0)    // Dont send empty message
                break;

            if (wcs_linebuf[0] == L'S') {
                wchar_t *sfn = OpenFileDialog();
        
                msg_sendfile( sfn );
                free(sfn);
                break;
            }
            
            message_t *sdmsg = msg_sendtext(wcs_linebuf);
            mm_scroll(sdmsg);
            mm_clearbuf();
            break;
        case '\b':
            if (lbuf_index)
                wcs_linebuf[--lbuf_index] = 0;
            mm_clearscr();
            break;
        default:
            if (lbuf_index < MAX_BODY)
                wcs_linebuf[lbuf_index++] = (wchar_t)ch;
            break;
    }
}
void mm_printlbuf(void) {
    SetConsoleCursorPosition(hOutput, (COORD) {0, 0});
    WriteConsoleW(hOutput, wcs_linebuf, lbuf_index, &written, NULL);
}

/* Normal functions */
void mm_scrint(void) {
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

void mm_clearscr(void) {
    FillConsoleOutputCharacter(hOutput, ' ', csbi.dwSize.X * csbi.dwSize.Y, (COORD){0, 0}, &written);
    FillConsoleOutputAttribute(hOutput, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, (COORD){0, 0}, &written);

    //FillConsoleOutputAttribute(hOutput, BACKGROUND_INTENSITY, s.dwSize.X, (COORD){0, 1}, &written);

    SetConsoleCursorPosition(hOutput, (COORD){0, 0});
}
void mm_scroll(message_t *new) {
    msg_free(msg_vector[0]);
    for (size_t i = 0; i < VECTOR_LENGTH; i++) {    // Add display limit depending on term resolution
        msg_vector[i] = msg_vector[i + 1];
    }
    msg_vector[VECTOR_LENGTH - 1] = new;
}

void mm_scrflush(void) {
    for (size_t i = 0; i < VECTOR_LENGTH; i++) {
        SetConsoleCursorPosition(hOutput, (COORD){0, START_LINE + (i * 2)});
        mm_msgformat(msg_vector[i]);
    }
}


int mm_kbdin(void) {
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

static void mm_msgformat(message_t *msg) {
    if (msg == NULL)
        return;

    if (*(msg->wcs_address)) {
        SetConsoleTextAttribute(hOutput, FOREGROUND_RED);
        WriteConsoleW( hOutput, L"PRIV ", 6, &written, NULL );
    }

    SetConsoleTextAttribute(hOutput, FOREGROUND_GREEN);
    WriteConsoleW(hOutput, msg->wcs_username, MAX_USERNAME, &written, NULL);   // I'm not sure MAX_USERNAME is correct here

    SetConsoleTextAttribute(hOutput, FOREGROUND_INTENSITY);
    WriteConsoleW(hOutput, L"> ", 3, &written, NULL);

    SetConsoleTextAttribute(hOutput, FOREGROUND_DEFAULT);

    BOOL shouldRender = TRUE;
    WORD wTextAttr = 0;
    wchar_t *privUser, *privUserPers;

    for (wchar_t *wch = msg->wcs_body; *wch; wch++) {
        switch (*wch) {
            case L'$':
                ++wch;
                switch (*wch) {
                    case L'$':
                        goto print;
                    case L'!':
                        SetConsoleTextAttribute(hOutput, FOREGROUND_DEFAULT);
                        wTextAttr = 0;
                        shouldRender = TRUE;
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

static void mm_clearbuf(void) {
    wmemset(wcs_linebuf, 0, MAX_BODY);
    lbuf_index = 0;
    mm_clearscr();
}