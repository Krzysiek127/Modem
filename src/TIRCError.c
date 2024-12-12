#include "TIRCError.h"


void _TIRCAssert(const wchar_t *text, const wchar_t *file, const uint32_t line) {
    wchar_t msg[256] = {0};
    swprintf(
        msg, 256,
        L"TIRC ASSERTION FAILURE\nIn %ls:%lu\nCause %ls\n",
        file, line, (text)
    );
    TIRCriticalError(msg);
}


void TIRCriticalError(const wchar_t *msg) {
    MessageBoxW(NULL, msg, PRG_NAME, MB_ICONERROR);
    WSACleanup();
    exit(1);
}


void TIRCFormatError(int lasterror) {
    wchar_t *s = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
        NULL, lasterror,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&s, 0, NULL
    );
    
    TIRCriticalError(s);
}