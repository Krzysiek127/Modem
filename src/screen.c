#include "screen.h"
#include <math.h>

#define START_LINE 3

HANDLE hInput, hOutput;

static int iTermWidth, iTermHeight;

static size_t iMsgQueueSz;
static message_t **msg_vector;  // I thought about switching to linked lists so that the message history would be possible

/* Forward declaration for static functions */
static void mm_msgformat(message_t *msg);

/* Normal functions */
void mm_scrint(void) {
    hInput = GetStdHandle(STD_INPUT_HANDLE);
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    TIRCAssert(hInput != NULL, L"Failed to initialize STDIN handle!");
    TIRCAssert(hOutput != NULL, L"Failed to initialize STDOUT handle!");

    //SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");
    SetConsoleMode(
        hInput, 
        ((~ENABLE_LINE_INPUT) & ~ENABLE_ECHO_INPUT) | ENABLE_PROCESSED_INPUT
    );


    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOutput, &csbi);
    iTermWidth = csbi.dwSize.X;
    iTermHeight = csbi.dwSize.Y;

    /* Determining how many chat lines are going to fit */
    iMsgQueueSz = (size_t)((iTermHeight - 3) / ceil((MAX_BODY + MAX_USERNAME + 2) / iTermWidth));
    msg_vector = calloc(iMsgQueueSz, sizeof(message_t*));
    /*
    3 for input line, toast line and linebreak then divide by:
    forst case scenario for a message dividied by terminal size

    example:
    Terminal is 80x24
        (24 - 3) / ceil(106 / 80)
        21 / ceil(>1)
        21 / 2
        10.5 => int(10)
    */
}

void mm_scroll(message_t *new) {
    msg_free(msg_vector[0]);
    for (size_t i = 0; i < iMsgQueueSz; i++) {
        msg_vector[i] = msg_vector[i + 1];
    }
    msg_vector[iMsgQueueSz - 1] = new;
}

void mm_scrflush(void) {
    for (size_t i = 0; i < iMsgQueueSz && msg_vector[i] != NULL; i++) {
        SetConsoleCursorPosition(hOutput, (COORD){0, START_LINE + (i * 2)});
        mm_msgformat(msg_vector[i]);
    }
}


int mm_kbdin(void) {
    TIRCAssert(hInput != NULL, L"Uninitialized STDIN handle!");
    INPUT_RECORD IR;
    DWORD EVENTSREAD;

    if (PeekConsoleInputW(hInput, &IR, 1, &EVENTSREAD) && EVENTSREAD) {
        ReadConsoleInputW(hInput, &IR, 1, &EVENTSREAD);
        return (IR.EventType == KEY_EVENT && IR.Event.KeyEvent.bKeyDown) ? IR.Event.KeyEvent.uChar.UnicodeChar : -1;
    }
    return -1;
}

static void mm_msgformat(message_t *msg) {
    if (msg == NULL)
        return;

    printf("%ls> %ls", msg->wcs_username, msg->wcs_body);   // For now
    return;
}