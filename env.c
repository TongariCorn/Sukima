#include "env.h"

void registerSymbol(Obj* _env, char* _key, Obj* _obj) {
	if (_env == NULL || _env->child == NULL || _key == NULL) return;

	Obj* frame = _env->child;
	Data data;
	data.name = NULL;
	Obj* nObj = NULL;
	if (_obj) nObj = _obj;
	else nObj = newObj(UNDEFINED, data);

	Obj* nKey = searchFrame(frame, _key);
	if (nKey == NULL) {
		data.name = malloc(strlen(_key)+1);
		strcpy(data.name, _key);
		nKey = newObj(SYMBOL, data);
		nKey->next = frame->child;
		frame->child = nKey;
	}

	nKey->child = nObj;
}

Obj* searchFrame(Obj* _frame, char* _key) {
	if (_frame == NULL || _key == NULL) return NULL;

	Obj* key = _frame->child;
	while (key) {
		if (strcmp(key->data.name, _key) == 0) {
			return key;
		}
		key = key->next;
	}
	return NULL;
}

Obj* nameResolution(Obj* _env, char* _key) {
	if (_env == NULL || _key == NULL) return NULL;

	Obj* frame = _env->child;
	while (frame) {
		Obj* key = searchFrame(frame, _key);
		if (key) return key;
		frame = frame->next;
	}
	return NULL;
}

Obj* createEnv(Obj* _parent_env) {
	Data data;
	data.num = 0;
	Obj* env = newObj(ENV, data);
	Obj* frame = newObj(FRAME, data);
	env->child = frame;

	if (_parent_env) frame->next = _parent_env->child;

	return env;
}

void pushEnv(Obj* _env) {
	if (_env == NULL) return;

	_env->next = getRoot()->child->child->child;
	getRoot()->child->child->child = _env;
}

void popEnv() {
	getRoot()->child->child->child = getRoot()->child->child->child->next;
}
