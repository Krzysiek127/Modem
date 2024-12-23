#ifndef H_SCREEN
#define H_SCREEN

#include "message.h"
#include "dlgbox.h"

// wtf does 'mm' mean???

void mm_screenInit(void);
void mm_screenClear(void);
void mm_screenFlush(void);

void mm_scroll(message_t *new);
void mm_kbdLine(void);
void mm_printLineBuff(void);

#endif