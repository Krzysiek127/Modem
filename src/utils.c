#include "include.h"

void TIRCriticalError(const wchar_t *msg) {
    MessageBoxW(NULL, msg, PRG_NAME, MB_ICONERROR);
    WSACleanup();
    exit(1);
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

    while (iswdigit(wcs[i]) && wcs[i] != 0) {
        ret = ret * 10 + (wcs[i] - L'0');
        ++i;
    }
    return ret;
}

wchar_t* wcs_copy_n(const wchar_t* source, const size_t n) {
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


// temporary (meaby)
inline bool hexdump(const void *addr, const size_t size, uint8_t perLine)
{
    if (!addr || !perLine)
        return true;

    if (perLine < 4) 
        perLine = 4;
    else if (perLine > 64)
        perLine = 64;

    printf("addr: %p\n", addr);

    unsigned char buff[perLine + 1];
    size_t i;
    for (i = 0; i < size; i++)
    {
        // Multiple of perLine means new or first line (with line offset).
        if (!(i % perLine))
        {
            // Only print previous-line ASCII buffer for lines beyond first.
            if (i) printf("  %s\n", buff);

            // Output the offset of current line.
            printf("%06zx:", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", ((unsigned char  *)addr)[i]);

        // And buffer a printable ASCII character for later.
        buff[i % perLine] = (isprint(((unsigned char  *)addr)[i]) ? ((unsigned char  *)addr)[i] : '.');
        
        buff[(i % perLine) + 1] = 0;
    }

    // Pad out last line if not exactly perLine characters.
    while ((i % perLine) != 0)
    {
        printf("   ");
        i++;
    }

    // And print the final ASCII buffer.
    printf("  %s\n", buff);
    return false;
}