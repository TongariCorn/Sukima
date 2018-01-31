#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "printError.h"
#include "obj.h"
#include "gc.h"

int isNumber(char s);
int isAlphabet(char s);
int parseObj(Obj** parent, char* str, int size);
int parser(Obj** parent, char* str);

#endif
