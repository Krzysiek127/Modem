#ifndef H_SCREEN
#define H_SCREEN

#include "include.h"
#include "message.h"
#include "dlgbox.h"

#define START_LINE 3
#define FOREGROUND_DEFAULT  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define MSG_BUFFER_SIZE   12

// wtf does 'mm' mean???

void mm_screenInit(void);
void mm_screenFlush(void);
void mm_clearScreen(void);

void mm_scroll(message_t *new);
void mm_kbdLine(void);
void mm_printHeader(void);

#endif