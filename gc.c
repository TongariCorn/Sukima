#include "gc.h"

static Obj* pool = NULL;
static int allocPtr = 0;
static Obj* root = NULL;
static int size = 0;

void initGC(int _size) {
	size = _size;
	pool = malloc(sizeof(Obj[size]));
	for (int i = 0; i < size; i++) {
		pool[i].type = UNDEFINED;
		pool[i].child = NULL;
		pool[i].next = NULL;
		pool[i].gc_mark = 0;
	}
	allocPtr = 0;
};

void setRootObj(Obj* _root) {
	root = _root;
}

static void markObj(Obj* obj) {
	if (obj == NULL) return;
	if (obj->gc_mark) return;

	obj->gc_mark = 1;
	markObj(obj->child);
	markObj(obj->next);

	if (obj->type == ENV_MEMORY) markObj(obj->data.p_env);
}

static void sweepObj() {
	for (int i = 0; i < size; i++) {
		if (pool[i].gc_mark == 0) {
			if (pool[i].type == STRING || pool[i].type == SYMBOL || pool[i].type == PROCEDURE || pool[i].type == BUILT_IN) {
				if (pool[i].data.name != NULL) {
					free(pool[i].data.name);
					pool[i].data.name = NULL;
				}
			}
			pool[i].type = UNDEFINED;
		}
	}
}

static void runGC() {
	for (int i = 0; i < size; i++) pool[i].gc_mark = 0;
	allocPtr = 0;

	if (root == NULL) return;

	// mark
	markObj(root->child);

	// sweep
	sweepObj();
}

Obj* newObj(enum TYPE type, Data data) {
	if (pool == NULL) return NULL;
	Obj* obj = NULL;

	int flag = 0;
	for (; allocPtr < size; allocPtr++) {
		if (pool[allocPtr].gc_mark == 0) {
			flag = 1;
			break;
		}
	}

	if (flag == 0) {
		runGC();
		
		for (; allocPtr < size; allocPtr++) {
			if (pool[allocPtr].gc_mark == 0) {
				flag = 1;
				break;
			}
		}
		if (flag == 0) return NULL;
	}

	obj = pool + allocPtr;
	obj->type = type;
	obj->data = data;
	obj->child = NULL;
	obj->next = NULL;
	obj->gc_mark = 1;
	return obj;
}

void freeObj() {
	sweepObj();
	free(pool);
}

void printGC() {
	printf("pool size:%d\n", size);
	printf("allocPtr:%d\n", allocPtr);
}

int getSize() {
	int s = 0;
	runGC();
	for (int i = allocPtr; i < size; i++) {
		if (pool[i].gc_mark == 0) s++;
	}
	return s;
}

int sizeCheck(int _size) {
	int flag = 0;
	int empty_size = 0;
	for (int i = allocPtr; i < size; i++) {
		if (pool[i].gc_mark == 0) empty_size++;
		if (empty_size >= _size) {
			flag = 1;
			break;
		}
	}

	if (flag == 0) {
		runGC();

		for (int i = allocPtr; i < size; i++) {
			if (pool[i].gc_mark == 0) empty_size++;
			if (empty_size >= _size) {
				flag = 1;
				break;
			}
		}
	}
	return flag;
}

static void clearCopyTo(Obj* obj) {
	if (obj == NULL) return;
	obj->copyTo = NULL;
	clearCopyTo(obj->next);
	clearCopyTo(obj->child);
} 

static int _copyObj(Obj** copyTo, Obj* source) {
	if (source == NULL) return 0;
	if (copyTo == NULL || source->type == ENV) return -1;
	
	if (source->copyTo) {	// 二重参照対策
		*copyTo = source->copyTo;
		return 0;
	}

	Obj* obj = newObj(source->type, source->data);
	obj->child = source->child;
	obj->next = source->next;
	if (source->type == STRING || source->type == SYMBOL || source->type == PROCEDURE || source->type == BUILT_IN) {
		if (source->data.name != NULL) {
			obj->data.name = malloc(strlen(source->data.name)+1);
			strcpy(obj->data.name, source->data.name);
		}
	}
	*copyTo = obj;
	source->copyTo = obj;
	if (_copyObj(&(*copyTo)->child, source->child) == -1) return -1;
	if (_copyObj(&(*copyTo)->next, source->next) == -1) return -1;

	return 0;
}

int copyObj(Obj** copyTo, Obj* source) {
	clearCopyTo(source);
	return _copyObj(copyTo, source);
}
