#include "core.h"

static Obj* root = NULL;
static Obj* stack = NULL;
static Obj* reg = NULL;

void initCore() {
	initError();
	initGC(4096);
	root = malloc(sizeof(Obj));
	root->next = NULL;
	root->type = ROOT;
	setRootObj(root);
	Data data;
	data.num = 0;
	stack = newObj(STACK, data);
	root->child = stack;
	reg = newObj(REGISTER, data);
	stack->child = reg;
	reg->child = newObj(ENV, data);
	Obj* frame = newObj(FRAME, data);
	reg->child->child = frame;
}

void endCore() {
	freeObj();
	if (root) free(root);
	deleteError();
}

Obj* getRoot() {
	return root;
}

Obj** getRegister() {
	return &(reg->next);
}

Obj* getEnv() {
	return stack->child->child;
}

Obj* getParentStack() {
	if (stack->next == NULL) return NULL;
	return stack->next->next;
}

Obj* getLastStack() {
	if (stack->next == NULL) return NULL;
	return stack->next;
}

void pushStack(Obj* expression) {
	Data data;
	data.num = 0;
	Obj* newStack = newObj(STACK, data);
	newStack->next = stack->next;
	stack->next = newStack;
	newStack->child = expression;
}

void popStack() {
	if (stack->next == NULL) return;
	stack->next = stack->next->next;
}

void replaceStack(Obj* newObj) {
	if (newObj == getLastStack()->child) return;

	if (newObj == NULL) {	// getLastStackを消してgetLastStack()->nextに進む
		Obj* obj = NULL;
		if (getParentStack() == NULL) {
			obj = stack->next->child;
			if (stack->next->child == getLastStack()->child) {
				stack->next->child = obj->next;
				getLastStack()->child = obj->next;
				return;
			}
		} else {
			obj = getParentStack()->child->child;
			if (obj == getLastStack()->child) {
				getParentStack()->child->child = obj->next;
				getLastStack()->child = obj->next;
				return;
			}
		}

		while (obj) {
			if (obj->next == getLastStack()->child) {
				obj->next = obj->next->next;
				getLastStack()->child = obj->next;
				return;
			}
			obj = obj->next;
		}
	} else {	// 通常のreplaceStack (自分の場所にnewObjを挿入)

		Obj *last = newObj;
		while (last->next) last = last->next;

		Obj *obj = NULL;
		if (getParentStack() == NULL) {        // Stackの親がいなかった場合 (Stack->next->next == NULL)
			obj = stack->next->child;
			if (stack->next->child == getLastStack()->child) {
				last->next = obj->next;
				stack->next->child = newObj;
				return;
			}
		} else {
			obj = getParentStack()->child->child;
			if (obj == getLastStack()->child) {
				last->next = obj->next;
				getParentStack()->child->child = newObj;
				getLastStack()->child = newObj;
				return;
			}
		}

		while (obj) {
			if (obj->next == getLastStack()->child) {
				last->next = obj->next->next;
				obj->next = newObj;
				getLastStack()->child = newObj;
				return;
			}
			obj = obj->next;
		}
	}
}

void proceedStack() {
	if (getLastStack()->child == NULL) return;

	getLastStack()->child = getLastStack()->child->next;
}
