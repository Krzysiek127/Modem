#ifndef H_TIRCERROR
#define H_TIRCERROR

#include "include.h"

// should not probably be called (used TIRCAssert macro)
void _TIRCAssert(const wchar_t *text, const wchar_t *file, const uint32_t line);

// displays messageBox and terminates program
void TIRCriticalError(const wchar_t *text);

void TIRCFormatError(int lasterror);


#define TIRCAssert(expr, text)  \
do {                            \
    if((expr)) {                \
        _TIRCAssert(            \
            (text),             \
            _CRT_WIDE(__FILE__),\
            __LINE__            \
        );                      \
    }                           \
} while (0)


#endif