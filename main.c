#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "printError.h"
#include "obj.h"
#include "gc.h"
#include "parser.h"
#include "core.h"
#include "env.h"
#include "sukima.h"

//====================================================================
//・二重参照(ex. A->C, B->C)は環境でのみしても良い
//・EnvMemoryのObj->dataもmark & sweepでマークする
//
// [全体像]
// Root (mark & sweepガベージコレクションのためのルートオブジェクト)
//  ↓
// Stack → Stack → Stack →
//  ↓       ↓
//  ↓      Expression
//  ↓
// Register (コピー用レジスタ)
//  ↓
// Env → Env →
//  ↓                       ↓
// Frame → Frame → Frame → Frame →
//  ↓
// Key → Key → Key →
//  ↓     ↓     ↓
// Obj   Obj   Obj
//
// [Procedureの仕様]
// Proc
//  ↓
// EnvMemory → Arguments
//  ↓
// Expression
//
//・EnvMemoryはObj->dataに環境へのポインタが入っている
//
// [Continuationの仕様]
//・call/ccで生成
//・FIELDから下の自分を含む式(Root->child->next->child)とEnv(Root->child->child)をコピーする
//・(cont var)で継続を実行、まずvarをRegisterに避難させ、StackにFIELDを設置したのちContの子であるStackとEnvをRoot以下に設置
//・
// Cont
//  ↓
// Stack → ...
//  ↓
// Env → ...
//
//====================================================================
int main() {
	/*initSukima();
	printf("Hello, World!\n");

	Data data;
	data.name = malloc(strlen("hello")+1);
	strcpy(data.name, "hello");
	Obj* test = newObj(STRING, data);
	registerSymbol(getEnv(), "test", *test);


	Obj* env = createEnv(getEnv());
	pushEnv(env);
	data.name = malloc(strlen("yeah")+1);
	strcpy(data.name, "yeah");
	Obj* yeah = newObj(STRING, data);
	registerSymbol(getEnv(), "hoo", *yeah);

	execSukima("(define (sum n c) (if (< n 1) c (sum (- n 1) (+ n c))))");
	execSukima("(sum 50000 0)");
	execSukima("(define (fix2 f)\n"
					   "((lambda (g) (f (lambda (n) ((g g) n))))\n"
					   "(lambda (g) (f (lambda (n) ((g g) n))))))\n"
					   "\n"
					   "(define (fact6 n)\n"
					   "(let ((f (lambda (g) (lambda (x)\n"
					   "  (if (< x 1)\n"
					   "  1\n"
					   "  (* (g (- x 1)) x))))))\n"
					   "((fix2 f) n))) (fact6 5)");

	printAllObj(getRoot(), 0);
	popEnv();

	endSukima();*/


	// エディタ機能付きSukima
	initSukima();
	int p_flag = -1; 	// 括弧の数が+-0になったら処理をする、-1は入力し始めのときのみ
	char buffer[4096];
	int buffer_len = 0;
	char str[256];
	int debug_flag = 0;
	while (1) {
		if (p_flag == -1) {
			printf("Sukima>> ");
			p_flag = 0;
			buffer_len = 0;
			memset(buffer, 0, sizeof(buffer));
		}

		for (int i = 0; i < p_flag; i++) printf("    ");
		fgets(str, sizeof(str)-1, stdin);

		if (strstr(str, "(quit)")) break;

		if (strstr(str, "(debug #t)")) {
			debug_flag = 1;
			p_flag = -1;
			continue;
		} else if (strstr(str, "(debug #f)")) {
			debug_flag = 0;
			p_flag = -1;
			continue;
		}

		if (strstr(str, "(size)")) {
			printf("gc size check:%d\n", sizeCheck(3000));
			p_flag = -1;
			continue;
		}

		int len = strlen(str);
		for (int i = 0; i < len; i++) {
			if (str[i] == '(') p_flag++;
			if (str[i] == ')') p_flag--;
		}

		// とじ括弧の数が多すぎる場合
		if (p_flag < 0) {
			printf("=> Parentheses Error\n");
			p_flag = -1;
			continue;
		}

		// バッファに収まりきらない場合
		if (buffer_len + strlen(str) + 1 >= sizeof(buffer)) {
			printf("=> code too large\n");
			p_flag = -1;
			continue;
		}
		strcat(buffer, str);
		strcat(buffer, " ");
		buffer_len += strlen(str);

		if (p_flag == 0) {
			if (execSukima(buffer) == -1) {
				// エラー処理
				printf("=> %s\n", getError());
				deleteError();
				initError();
				p_flag = -1;
				continue;
			}

			if (getRoot()->child->next->child != NULL) {
				Obj* result = getRoot()->child->next->child; // FIELD
				if (result->child) {
					if (result->child->type == EXPRESSION) {
						char r[1024];
						if (printObjContent(result, r, 0, 1024) == -1) break;
						if (r[0] != '\0') {
							printf("=> ");
							printf("%s", r);
						}
					} else {
						result = result->child;
						while (result) {
							printf("=> ");
							printObj(result, 1, 0);
							result = result->next;
						}
					}
				}
			}
			p_flag = -1;

			if (debug_flag) printAllObj(getRoot(), 0);
		}
	}
	endSukima();
	return 0;
}
