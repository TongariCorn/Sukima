#include "parser.h"

static char exAlphabet[] = {'!', '$', '&', '*', '+', '-', '.', '/', ':', '<', '=', '>', '\?', '@', '^', '_', '~'};

int isNumber(char s) {
	if ('0' <= s && s <= '9') return 1;
	return 0;
}

int isAlphabet(char s) {
	if (('a' <= s && s <= 'z') || ('A' <= s && s <= 'Z')) return 1;
	for (int i = 0; i < sizeof(exAlphabet) / sizeof(char); i++) {
		if (s == exAlphabet[i]) return 1;
	}
	return 0;
}

int parseObj(Obj** parent, char* str, int size) {
	if (parent == NULL || size <= 0) return 0;

	int i = 0;
	while (1) {
		if (i >= size) return size;

		if (str[i] == '(') {
			Data data;
			data.name = NULL;
			*parent = newObj(EXPRESSION, data);
			i++;
			Obj** obj = &(*parent)->child;
			while (1) {
				int result = parseObj(obj, str + i, size - i);
				if (result == -1) return -1;
				i += result;

				if (i == size) {
					printError("Expression has to end with ).");
					return -1;
				} else if (str[i] == ')') return i+1;

				if (*obj) obj = &(*obj)->next;
			}
		}

		if (str[i] == ')') return i;

		if (str[i] == '\'') {
			Data data;
			data.name = NULL;
			*parent = newObj(QUOTE, data);
			i++;
			int result = parseObj(&(*parent)->child, str+i, size-i);
			i += result;
			return i;
		}

		if (isNumber(str[i])) {
			int j = 1;
			while (1) {
				if (str[i+j] == ' ' || str[i+j] == '\t'|| str[i+j] == '\n' || str[i+j] == '\0' || str[i+j] == '(' || str[i+j] == ')') {
					char* num = malloc(sizeof(char[j+1]));
					memcpy(num, str+i, sizeof(char[j]));
					num[j] = '\0';

					Data data;
					data.num = atof(num);
					free(num);
					*parent = newObj(NUMBER, data);
					i += j;
					return i;
				}

				if (!(isNumber(str[i+j]) || str[i+j] == '.')) break;

				j++;
			}
		}

		if (isAlphabet(str[i])) {
			int j = 1;
			while (1) {
				if (str[i+j] == ' ' || str[i+j] == '\t'|| str[i+j] == '\n' || str[i+j] == '\0' || str[i+j] == '(' || str[i+j] == ')') {
					char* name = malloc(sizeof(char[j+1]));
					memcpy(name, str+i, sizeof(char[j]));
					name[j] = '\0';
					Data data;
					data.name = name;
					*parent = newObj(SYMBOL, data);

					i += j;
					return i;
				}

				if (!(isAlphabet(str[i+j]) || isNumber(str[i+j]))) break;

				j++;
			}
		}

		if (str[i] == '\"') {
			int j = 1;
			while (1) {
				if (i+j == size) {
					printError("couldn't find \".\n");
					return -1;
				}
				if (str[i+j] == '\"') {
					char* name = malloc(sizeof(char[j]));
					memcpy(name, str+i+1, sizeof(char[j-1]));
					name[j] = '\0';

					Data data;
					data.name = name;
					*parent = newObj(STRING, data);
					i += j + 1;
					return i;
				}
				j++;
			}
		}

		if (str[i] == '#') {
			if (str[i+1] == 't') {
				Data data;
				data.boolean = 1;
				*parent = newObj(BOOLEAN, data);
				i += 2;
				return i;
			}

			if (str[i+1] == 'f') {
				Data data;
				data.boolean = 0;
				*parent = newObj(BOOLEAN, data);
				i += 2;
				return i;
			}
		}

		i++;
	}
}

int parser(Obj** parent, char* str) {
	Data data;
	data.name = NULL;
	Obj* obj = newObj(FIELD, data);
	*parent = obj;
	Obj** now = &(*parent)->child;

	int size = strlen(str)+1;
	int i = 0;
	while (1) {
		int result = parseObj(now, str + i, size - i);
		if (result == -1) return -1;
		i += result;

		if (result == 0 && str[i] == ')') {
			printError("too many parentheses exist.");

		}
		if (i == size) return size;

		if (*now) now = &(*now)->next;
	}
}
