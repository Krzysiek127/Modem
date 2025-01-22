#include "include.h"

extern void sock_cleanup(void);
extern void mm_cleanup(void);

void safeExit(void) {
    mm_cleanup();
    sock_cleanup();
    exit(0);
}

void TIRCriticalError(const wchar_t *msg) {
    wchar_t errMsg[255];
    swprintf(errMsg, 255, L"%ls\nlast error id: %d", msg, GetLastError());
    MessageBoxW(NULL, errMsg, PRG_NAME, MB_ICONERROR);
    safeExit();
}

// Maximum format length is 230
void MessageBoxW_Format(const wchar_t *format, ...) {
    va_list argptr;

    if (wcslen(format) > 230)
        return;
    
    wchar_t buffer[256];
    
    va_start(argptr, format);
    vswprintf(buffer, 256, format, argptr);
    va_end(argptr);

    MessageBoxW(NULL, buffer, PRG_NAME, 0);
}

void TIRCFormatError(int lasterror) {
    wchar_t *s = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
        NULL,
        (DWORD)lasterror,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&s, 0, NULL
    );
    
    TIRCriticalError(s);
}

uint32_t wcstou32(const wchar_t *wcs) {
    uint32_t i = 0, ret = 0;

    while (iswdigit(wcs[i]) && wcs[i] != 0 && i < 10) {
        ret = ret * 10 + (wcs[i] - L'0');
        ++i;
    }
    return ret;
}

void *memcpy_n(const void *restrict source, const size_t size) {
    if (source == NULL)
        return NULL;
        
    unsigned char *tmp = malloc(size + 1);

    if (tmp != NULL)
        memcpy(tmp, source, size);
    
    return tmp;
}

size_t wcs_scan(const wchar_t *src) {
    size_t len = 0;
    while (!iswspace(src[len++]));
    return len;
}