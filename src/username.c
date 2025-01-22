#include "username.h"

extern wchar_t wcs_currentUser[MAX_USERNAME];

void createUser(void) {
    wprintf(L"Username: ");

    TIRCAssert(
        fgetws(wcs_currentUser, MAX_USERNAME, stdin) == NULL,
        L"Invalid username"
    );

    // remove ungraph-able chars (if there are any)
    for (wchar_t *wcsptr = wcs_currentUser; *wcsptr; wcsptr++) {
        if (!iswgraph(*wcsptr))
            *wcsptr = 0;
    }

    FILE *fUserFile = fopen(USER_FILE, "wb");

    TIRCAssert(
        fUserFile == NULL,
        L"Username file could not be read"
    );

    fwrite(wcs_currentUser, sizeof(wchar_t), MAX_USERNAME, fUserFile);
    fclose(fUserFile);
}


void readUser(void) {
    if (fileExists("username")) {
        FILE *fUserFile = fopen(USER_FILE, "rb");

        TIRCAssert(
            fUserFile == NULL,
            L"Username file could not be read"
        );
        
        fseek(fUserFile, 0L, SEEK_END);

        TIRCAssert(
            ftell(fUserFile) != (MAX_USERNAME * sizeof(wchar_t)),
            L"Invalid username stored! Delete the file"
        );
        
        rewind(fUserFile);
        fread(wcs_currentUser, sizeof(wchar_t), MAX_USERNAME, fUserFile);
        fclose(fUserFile);
        
        return;
    }

    wprintf(L"No username file found. Creating new file.\n");
    createUser();
}