#include "printError.h"

static char* buffer = NULL;
static int b_nowAt = 0;

void initError() {
	buffer = calloc(1024, sizeof(char));
	b_nowAt = 0;
}

void printError(char* error) {
	if (error == NULL || buffer == NULL) return;
	int length = strlen(error);
	if (b_nowAt + length >= 1024) return;
	strcpy(buffer+b_nowAt, error);
	buffer[b_nowAt + length + 1] = '\n';
	b_nowAt += length+2;
}

char* getError() {
	return buffer;
}

void deleteError() {
	if (buffer) free(buffer);
}
