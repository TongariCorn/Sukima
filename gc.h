#ifndef _GC_H_
#define _GC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj.h"

void initGC(int _size);
void setRootObj(Obj* _root);
Obj* newObj(enum TYPE type, Data data);
void freeObj();
void printGC();

int getSize();
int sizeCheck(int _size);
int copyObj(Obj** copyTo, Obj* source);
#endif
