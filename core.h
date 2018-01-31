#ifndef _CORE_H_
#define _CORE_H_

#include <stdlib.h>

#include "printError.h"
#include "obj.h"
#include "gc.h"

void initCore();
void endCore();

Obj* getRoot();
Obj** getRegister();
Obj* getEnv();

Obj* getParentStack();
Obj* getLastStack();
void pushStack(Obj* expression);
void popStack();

void replaceStack(Obj* newObj);
void proceedStack();

int createContinuation(Obj** copyTo);

#endif
