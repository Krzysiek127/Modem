#ifndef H_USER
#define H_USER

#include "include.h"

#define USER_FILE ("username")

#define fileExists(fileName) (_access((fileName), F_OK))

// prompts for username file creation and sets current user
void createUser(void);

// sets current user from local file (if it does not exist calls createUser())
void readUser(void);

#endif