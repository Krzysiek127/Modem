#ifndef H_SCREEN
#define H_SCREEN

#include "message.h"

void mm_scrint(void);
void mm_scroll(message_t *new);
void mm_scrflush(void);
int mm_kbdin(void);
void mm_kbdline(void);
void mm_clearscr(void);
void mm_printlbuf(void);

#endif