#include "obj.h"

void printObj(Obj* obj, int simple, int level) {
	if (obj == NULL) return;
	for (int i = 0; i < level; i++) printf("    ");
	switch(obj->type) {
		case FIELD:
			printf("[FIELD]   (%p)\n", obj);
			break;

		case EXPRESSION:
			printf("[EXPRESSION]   (%p)\n", obj);
			break;

		case QUOTE:
			printf("[QUOTE]   (%p)\n", obj);
			break;

		case SYMBOL:
			printf("[SYMBOL]%s   (%p)\n", obj->data.name, obj);
			break;

		case PROCEDURE:
			printf("[PROCEDURE]%s   (%p)\n", obj->data.name, obj);
			break;

		case NUMBER:
			if (simple) printf("%f\n", obj->data.num);
			else printf("[NUMBER]%f   (%p)\n", obj->data.num, obj);
			break;

		case STRING:
			if (simple) printf("%s\n", obj->data.name);
			else printf("[STRING]%s   (%p)\n", obj->data.name, obj);
			break;

		case BOOLEAN:
			if (simple) {
				if (obj->data.boolean) printf("#t\n");
				else printf("#f\n");
			} else {
				if (obj->data.boolean) printf("[BOOLEAN]#t   (%p)\n", obj);
				else printf("[BOOLEAN]#f   (%p)\n", obj);
			}
			break;

		case CONTINUATION:
			printf("[CONTINUATION]   (%p)", obj);
			break;

		case ROOT:
			printf("[ROOT]   (%p)\n", obj);
			break;

		case REGISTER:
			printf("[REGISTER]   (%p)\n", obj);
			break;

		case STACK:
			printf("[STACK]   (%p)\n", obj);
			break;

		case ENV:
			printf("[ENV]   (%p)\n", obj);
			break;

		case FRAME:
			printf("[FRAME]   (%p)\n", obj);
			break;

		case ENV_MEMORY:
			printf("[ENV_MEMORY]   (%p)\n", obj);
			break;

		case POP_ENV_BLOCK:
			printf("[POP_ENV_BLOCK]   (%p)\n", obj);
			break;

		case BUILT_IN:
			printf("[BUILT_IN]%s   (%p)\n", obj->data.name, obj);
			break;

		default:
			printf("[UNDEFINED]   (%p)\n", obj);
	}
}

void printAllObj(Obj* obj, int level) {
	if (obj == NULL) return;
	printObj(obj, 0, level);
	printAllObj(obj->child, level+1);
	printAllObj(obj->next, level);
}

int printObjContent(Obj* obj, char* str, int i, int length) {
	if (obj == NULL) return 0;
	if (i >= length) return -1;
	switch (obj->type) {
		case FIELD: {
			Obj* child = obj->child;
			while (child) {
				i = printObjContent(child, str, i, length);
				if (i == -1 || i >= length) return -1;
				str[i] = '\n';
				i++;
				child = child->next;
			}
			break;
		}

		case EXPRESSION: {
			str[i] = '(';
			i++;
			Obj* child = obj->child;
			while (child) {
				i = printObjContent(child, str, i, length);
				if (i == -1 || i >= length) return -1;
				if (child->next) {
					str[i] = ' ';
					i++;
					child = child->next;
				} else {
					break;
				}
			}
			str[i] = ')';
			i++;
			break;
		}

		case QUOTE: {
			if (i+2 >= length) return -1;
			str[i] = '\'';
			str[i+1] = '(';
			i += 2;
			Obj* child = obj->child;
			while (child) {
				i = printObjContent(child, str, i, length);
				if (i == -1 || i >= length) return -1;
				if (child->next) {
					str[i] = ' ';
					i++;
					child = child->next;
				} else {
					break;
				}
			}
			str[i] = ')';
			i++;
			break;
		}

		case SYMBOL: {
			if (obj->data.name == NULL) break;
			int len = strlen(obj->data.name);
			if (i + len >= length) return -1;
			strcpy(str+i, obj->data.name);
			i += len;
			break;
		}

		case CONTINUATION:
		case BUILT_IN:
			if (obj->data.name) {
				int len = strlen(obj->data.name);
				if (i + len >= length) return -1;
				strcpy(str+i, obj->data.name);
				i += len;
			}
			break;

		case PROCEDURE: {
			if (obj->data.name) {
				int len = strlen(obj->data.name);
				if (i + len >= length) return -1;
				strcpy(str+i, obj->data.name);
				i += len;
			} else {
				if (i+15 >= length) return -1;
				strcpy(str+i, "(lambda (");
				i += strlen("(lambda (");
				Obj* arg = obj->child->next;
				while (arg) {
					if (arg->type != SYMBOL) return -1;
					int len = strlen(arg->data.name);
					strcpy(str+i, arg->data.name);
					i += len;
					if (arg->next) {
						if (i >= length) return -1;
						str[i] = ' ';
						i++;
						arg = arg->next;
					} else {
						if (i >= length) return -1;
						break;
					}
				}
				str[i] = ')';
				str[i+1] = ' ';
				i += 2;
				i = printObjContent(obj->child->child, str, i, length);
				if (i >= length) return -1;
				str[i] = ')';
				i++;
			}
			break;
		}

		case NUMBER: {
			char num[12];
			snprintf(num, 12, "%f", obj->data.num);
			int len = strlen(num);
			if (i + len >= length) return -1;
			strcpy(str+i, num);
			i += len;
			break;
		}

		case STRING: {
			int len = strlen(obj->data.name);
			if (i+2+len >= length) return -1;
			str[i] = '\"';
			i++;
			strcpy(str+i, obj->data.name);
			i += len;
			str[i] = '\"';
			i++;
			break;
		}

		case BOOLEAN:
			if (i+2 >= length) return -1;
			str[i] = '#';
			i++;
			if (obj->data.boolean) str[i] = 't';
			else str[i] = 'f';
			i++;
			break;

		default:
			;
	}
	if (i >= length) return -1;
	str[i] = '\0';
	return i;
}
