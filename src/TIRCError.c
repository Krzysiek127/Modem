#include "include.h"


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