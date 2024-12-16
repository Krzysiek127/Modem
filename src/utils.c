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

uint32_t wcstou32(wchar_t *wcs) {
    uint32_t i = 0, ret = 0;

    while (iswdigit(wcs[i]) && wcs[i] != 0) {
        ret = ret * 10 + (wcs[i] - L'0');
        ++i;
    }
    return ret;
}

wchar_t* wcs_copy_n(const wchar_t* source, size_t n) {
    if (!source)
        return NULL;
        
    wchar_t *tmp = calloc(n + 1, sizeof(wchar_t));
    wcsncpy(tmp, source, n);
    return tmp;
}

size_t wcs_scan(const wchar_t *src) {
    size_t len = 0;
    while (!iswspace(src[len++]));
    return len;
}