#ifndef _OBJ_H_
#define _OBJ_H_

#include <stdio.h>
#include <string.h>

enum TYPE{
	UNDEFINED,
	FIELD,
	EXPRESSION,
	QUOTE,
	SYMBOL,
	NUMBER,
	STRING,
	BOOLEAN,
	PROCEDURE,
	CONTINUATION,

	ROOT,
	REGISTER,
	STACK,
	ENV,
	FRAME,
	ENV_MEMORY,
	POP_ENV_BLOCK,
	BUILT_IN
};

typedef union {
	void* p_env;	// ENV_MEMORY
	char* name; 	// SYMBOL, STRING
	double num;	// NUMBER
	int boolean;	// BOOLEAN
}Data;

typedef struct Object{
	enum TYPE type;
	Data data;
	struct Object* child;
	struct Object* next;
	struct Object* copyTo;	// used by copyObj()
	short gc_mark;
}Obj;

void printObj(Obj* obj, int simple, int level);
void printAllObj(Obj* obj, int level);
int printObjContent(Obj* obj, char* str, int i, int length);

#endif
