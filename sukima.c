#include "sukima.h"

void initSukima() {
	initCore();
}

void endSukima() {
	endCore();
}

int execSukima(char* program) {
	pushStack(NULL);
	getLastStack()->next = NULL;
	if (parser(&getLastStack()->child, program) == -1) return -1;
	if (eval() == -1) return -1;
	return 0;
}
