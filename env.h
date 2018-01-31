#ifndef _ENV_H_
#define _ENV_H_

#include <string.h>

#include "printError.h"
#include "obj.h"
#include "gc.h"
#include "core.h"

void registerSymbol(Obj* _env, char* _key, Obj* _obj);
Obj* searchFrame(Obj* _frame, char* _key);
Obj* nameResolution(Obj* _env, char* _key);

Obj* createEnv(Obj* _parent_env);
void pushEnv(Obj* _env);
void popEnv();

#endif
