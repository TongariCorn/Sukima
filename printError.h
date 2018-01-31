#ifndef _PRINTERROR_H_
#define _PRINTERROR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initError();
void printError(char* error);
char* getError();
void deleteError();

#endif
