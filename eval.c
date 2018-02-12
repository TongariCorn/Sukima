#include "eval.h"
#include "obj.h"

#define GENERATE_BUILT_IN(NAME) {\
if (strcmp(NAME, getLastStack()->child->data.name) == 0) {\
Data data; \
data.name = malloc(strlen(NAME)+1); \
strcpy(data.name, NAME); \
obj = newObj(BUILT_IN, data); \
break;}}

#define APPLY_BUILT_IN(NAME, FUNCTION) {\
if (strcmp(getLastStack()->child->child->data.name, NAME) == 0) {\
if (FUNCTION == -1) return -1;\
continue;\
}}

int eval_define(Obj** obj) {
	if (getLastStack()->child->next == NULL) {
		printError("bad from.");
		return -1;
	}

	if (getLastStack()->child->next->type == EXPRESSION) {	// define-function
		// (define form body*)
		if (getLastStack()->child->next->next == NULL) {
			printError("bad form.");
			return -1;
		}
		// (define (SYMBOL ...) expression*)
		Obj* child = getLastStack()->child->next->child;
		if (!(child != NULL && child->type == SYMBOL)) {
			printError("bad form.");
			return -1;
		}
		child = child->next;
		while (child) {
			if (child->type != SYMBOL) {
				printError("bad form.");
				return -1;
			}
			child = child->next;
		}

		Data data;
		data.name = NULL;
		Obj* quote1 = newObj(QUOTE, data);
		quote1->child = getLastStack()->child->next;
		quote1->next = getLastStack()->child->next->next;
		getLastStack()->child->next = quote1;
		quote1->child->next = NULL;

		Obj* last = getLastStack()->child->next->next;
		while (last->next) {last = last->next;}
		Obj* quote2 = newObj(QUOTE, data);
		quote2->child = last;
		getLastStack()->child->next->next = quote2;

		data.name = malloc(strlen("define-func")+1);
		strcpy(data.name, "define-func");
		*obj = newObj(BUILT_IN, data);
		return 0;
	} else {	// define-variable
		if (getLastStack()->child->next->type != SYMBOL) {
			printError("bad form.");
			return -1;
		}
		Data data;
		data.name = NULL;
		Obj* quote = newObj(QUOTE, data);
		quote->child = getLastStack()->child->next;
		quote->next = getLastStack()->child->next->next;
		getLastStack()->child->next = quote;
		quote->child->next = NULL;

		data.name = malloc(strlen("define-var")+1);
		strcpy(data.name, "define-var");
		*obj = newObj(BUILT_IN, data);
		return 0;
	}
}

int eval_if(Obj** obj) {
	Obj* now = getLastStack()->child;
	if (!(now->next != NULL && now->next->next != NULL)) {
		printError("bad form.");
		return -1;
	}

	Data data;
	data.name = NULL;
	Obj* quote = newObj(QUOTE, data);
	quote->child = now->next->next;
	quote->next = now->next->next->next;
	now->next->next = quote;
	quote->child->next = NULL;

	if (now->next->next->next) {
		if (now->next->next->next->next != NULL) {
			printError("bad form.");
			return -1;
		}

		Obj* quote2 = newObj(QUOTE, data);
		quote2->child = now->next->next->next;
		now->next->next->next = quote2;
	}

	data.name = malloc(strlen("if")+1);
	strcpy(data.name, "if");
	*obj = newObj(BUILT_IN, data);

	return 0;
}

int eval_lambda(Obj** obj) {
	// (lambda (SYMBOL SYMBOL ...) PROC_CONTENT*)
	Obj* now = getLastStack()->child;
	if (!(now->next != NULL && now->next->type == EXPRESSION && now->next->next != NULL)) {
		printError("bad form.");
		return -1;
	}

	Obj* arg = now->next->child;
	while (arg) {
		if (arg->type != SYMBOL) {
			printError("bad form.");
			return -1;
		}
		arg = arg->next;
	}

	Data data;
	data.name = NULL;
	Obj* quote1 = newObj(QUOTE, data);
	quote1->child = now->next;
	quote1->next = now->next->next;
	now->next = quote1;
	quote1->child->next = NULL;

	Obj* quote2 = newObj(QUOTE, data);
	quote2->child = now->next->next;
	now->next->next = quote2;

	data.name = malloc(strlen("lambda")+1);
	strcpy(data.name, "lambda");
	*obj = newObj(BUILT_IN, data);
	return 0;
}

int eval_let(Obj** obj) {
	Obj* now = getLastStack()->child;
	if (!(now->next != NULL && now->next->type == EXPRESSION && now->next->next != NULL)) {
		printError("bad form.");
		return -1;
	}

	//==================================================================
	// (let ((Symbol1 exp1) (Symbol2 exp2) ...) expression) を
	// (Proc exp1 exp2 ...) に変換
	//
	// ここでProcは、現在の環境を保存し、引数にSymbol1, Symbol2, ...をとり、expressionを実行する無名関数

	// letをprocに変換
	// (proc ((Symbol1 exp1) (Symbol2 exp2) ...) expression)
	//==================================================================
	if (now->data.name) free(now->data.name);
	now->data.name = NULL;
	now->type = PROCEDURE;

	// procの下にENV_MEMORYを設定
	Data data;
	data.p_env = getEnv();
	Obj* env_memory = newObj(ENV_MEMORY, data);
	now->child = env_memory;

	// expressionをENV_MEMORYの下に移動させる
	env_memory->child = now->next->next;
	now->next->next = NULL;

	//==================================================================
	// (Symbol1 exp1) (Symbol2 exp2) ... を一度ENV_MEMORYの隣に移動させる
	// (Proc)
	//   |
	//  ENV_MEMORY -> (Symbol1 exp1) -> (Symbol2 exp2) -> ...
	//   |
	//  expression
	//==================================================================
	env_memory->next = now->next->child;
	now->next = NULL;

	//==================================================================
	// (Symbol1 exp1)をSymbol1に置き換え、exp1をProcの隣に移動させる
	// (Proc exp1 exp2 ...)
	//   |
	//  ENV_MEMORY -> Symbol1 -> Symbol2 -> ...
	//   |
	//  expression
	//==================================================================
	Obj* arg = env_memory;
	Obj* var = now;
	while (arg->next) {
		Obj* exp = arg->next;
		if (!(exp->type == EXPRESSION && exp->child != NULL && exp->child->type == SYMBOL && exp->child->next != NULL && exp->child->next->next == NULL)) {
			printError("bad form.");
			return -1;
		}
		var->next = arg->next->child->next;			// expをprocの隣に移動
		arg->next->child->next = arg->next->next;	// Symbolの隣を次の(Symbol exp)に繋げる
		arg->next = arg->next->child;				// (Symbol)の()を外す i.e. Symbol1(arg) -> Symbol2 -> (Symbol exp) -> ...

		arg = arg->next;
		var = var->next;
	}

	*obj = now;
	return 0;
}

int eval_set_ex(Obj** obj) {
	if (getLastStack()->child->next == NULL || getLastStack()->child->next->type != SYMBOL) {
		printError("bad form.");
		return -1;
	}
	Data data;
	data.name = NULL;
	Obj* quote = newObj(QUOTE, data);
	quote->child = getLastStack()->child->next;
	quote->next = getLastStack()->child->next->next;
	getLastStack()->child->next = quote;
	quote->child->next = NULL;

	data.name = malloc(strlen("set!")+1);
	strcpy(data.name, "set!");
	*obj = newObj(BUILT_IN, data);
	return 0;
}

int apply_add() {
	Obj* now = getLastStack()->child->child;
	if (!(now->next != NULL && now->next->type == NUMBER && now->next->next != NULL && now->next->next->type == NUMBER)) {
		printError("bad form.");
		return -1;
	}

	Data result;
	result.num = now->next->data.num + now->next->next->data.num;
	now = now->next->next->next;
	while (now) {
		if (now->type != NUMBER) {
			printError("bad form.");
			return -1;
		}
		result.num += now->data.num;
		now = now->next;
	}

	Obj* reObj = newObj(NUMBER, result);
	replaceStack(reObj);
	proceedStack();
	return 0;
}

int apply_sub() {
	Obj* now = getLastStack()->child->child;
	if (!(now->next != NULL && now->next->type == NUMBER && now->next->next != NULL && now->next->next->type == NUMBER)) {
		printError("bad form.");
		return -1;
	}

	Data result;
	result.num = now->next->data.num - now->next->next->data.num;
	now = now->next->next->next;
	while (now) {
		if (now->type != NUMBER) {
			printError("bad form.");
			return -1;
		}
		result.num -= now->data.num;
		now = now->next;
	}

	Obj* reObj = newObj(NUMBER, result);
	replaceStack(reObj);
	proceedStack();
	return 0;
}

int apply_mul() {
	Obj* now = getLastStack()->child->child;
	if (!(now->next != NULL && now->next->type == NUMBER && now->next->next != NULL && now->next->next->type == NUMBER)) {
		printError("bad form.");
		return -1;
	}

	Data result;
	result.num = now->next->data.num * now->next->next->data.num;
	now = now->next->next->next;
	while (now) {
		if (now->type != NUMBER) {
			printError("bad form.");
			return -1;
		}
		result.num *= now->data.num;
		now = now->next;
	}

	Obj* reObj = newObj(NUMBER, result);
	replaceStack(reObj);
	proceedStack();
	return 0;
}

int apply_less_than() {
	Data data;

	Obj* now = getLastStack()->child->child;
	if (now->next == NULL) goto TRUE;

	if (now->next->type != NUMBER) {
		printError("bad form.");
		return -1;
	}

	now = now->next;
	while (now->next) {
		if (now->next->type != NUMBER) {
			printError("bad form.");
			return -1;
		}

		if (now->data.num >= now->next->data.num) goto FALSE;
		now = now->next;
	}

	goto TRUE;

TRUE:
	data.boolean = 1;
	Obj* t = newObj(BOOLEAN, data);
	replaceStack(t);
	proceedStack();
	return 0;

FALSE:
	data.boolean = 0;
	Obj* f = newObj(BOOLEAN, data);
	replaceStack(f);
	proceedStack();
	return 0;
}

int apply_greater_than() {
	Data data;

	Obj* now = getLastStack()->child->child;
	if (now->next == NULL) goto TRUE;

	if (now->next->type != NUMBER) {
		printError("bad form.");
		return -1;
	}

	now = now->next;
	while (now->next) {
		if (now->next->type != NUMBER) {
			printError("bad form.");
			return -1;
		}

		if (now->data.num <= now->next->data.num) goto FALSE;
		now = now->next;
	}

	goto TRUE;

TRUE:
	data.boolean = 1;
	Obj* t = newObj(BOOLEAN, data);
	replaceStack(t);
	proceedStack();
	return 0;

FALSE:
	data.boolean = 0;
	Obj* f = newObj(BOOLEAN, data);
	replaceStack(f);
	proceedStack();
	return 0;
}

int apply_define_var() {
	Obj* now = getLastStack()->child->child;
	registerSymbol(getEnv(), now->next->data.name, now->next->next);
	replaceStack(NULL);
	return 0;
}

int apply_define_func() {
	// (define (PROC_NAME SYMBOL SYMBOL ...) PROC_CONTENT)
	Obj* now = getLastStack()->child;
	Obj* exp = now->child->next;
	Data data;
	data.name = malloc(strlen(exp->child->data.name)+1);
	strcpy(data.name, exp->child->data.name);
	Obj* proc = newObj(PROCEDURE, data);
	proc->next = now->next;		// 一時的にnowの隣にprocをつけることでGCに回収されることを防ぎ、procの下にPROCEDUREを作っていく
	now->next = proc;

	// ENV_MEMORY
	data.p_env = getEnv();
	proc->child = newObj(ENV_MEMORY, data);

	// args
	proc->child->next = exp->child->next;

	// proc_content
	proc->child->child = exp->next;

	now->next = proc->next;
	proc->next = NULL;
	registerSymbol(getEnv(), proc->data.name, proc);
	replaceStack(NULL);
	return 0;
}

int apply_if() {
	// (if (condition) true) / (if (condition) true false)
	Obj* now = getLastStack()->child->child;
	if (!(now->next != NULL && now->next->next != NULL && now->next->type == BOOLEAN)) {
		printError("bad form.");
		return -1;
	}

	Obj* obj = NULL;
	if (now->next->data.boolean) {
		obj = now->next->next;
		obj->next = NULL;
	} else if (now->next->next->next != NULL) {
		obj = now->next->next->next;
		obj->next = NULL;
	}

	replaceStack(obj);
	return 0;
}

int apply_lambda() {
	// (lambda (SYMBOL SYMBOL ...) proc_content)
	Obj* now = getLastStack()->child;

	Data data;
	data.name = NULL;
	Obj* proc = newObj(PROCEDURE, data);
	proc->next = now->next;
	now->next = proc;	// nowの隣にprocを置くことでGCされるのを防ぐ

	// ENV_MEMORY
	data.p_env = getEnv();
	proc->child = newObj(ENV_MEMORY, data);

	// args
	Obj* args = now->child->next->child;
	proc->child->next = args;

	// proc_content
	proc->child->child = now->child->next->next;

	replaceStack(NULL);	// nowを外す
	proceedStack();
	return 0;
}

int apply_list() {
	Obj* exp = getLastStack()->child;
	exp->child = exp->child->next;
	proceedStack();
	return 0;
}

int apply_car() {
	Obj* now = getLastStack()->child->child;
	if (!(now->next != NULL && now->next->type == EXPRESSION && now->next->child != NULL)) {
		printError("bad form");
		return -1;
	}

	now->next->child->next = NULL;
	replaceStack(now->next->child);
	proceedStack();
	return 0;
}

int apply_cdr() {
	Obj* exp = getLastStack()->child;
	if (!(exp->child->next != NULL && exp->child->next->type == EXPRESSION && exp->child->next->child != NULL)) {
		printError("bad form");
		return -1;
	}

	exp->child = exp->child->next->child->next;
	proceedStack();
	return 0;
}

int apply_call_cc() {
	// (call/cc PROCEDURE)
	// PROCEDUREは引数を1つもつ
	Obj* now = getLastStack()->child->child;

	if (!(now->next && now->next->type == PROCEDURE && now->next->next == NULL)) {
		printError("bad form.");
		return -1;
	}

	// 継続の作成
	// Cont
	//  ↓
	// Stack → ...
	//  ↓
	// Env → ...
	Data data;
	data.name = NULL;
	Obj* cont = newObj(CONTINUATION, data);
	*getRegister() = cont;
	cont->child = newObj(STACK, data);
	
	copyObj(&(cont->child->next), getLastStack());
	copyObj(&(cont->child->child), getEnv());

	// FILED以下の実行中以外のコードを消す
	Obj* lastStack = cont->child;
	while (lastStack->next->next) { lastStack = lastStack->next; }
	// -> Stack -> Stack
	//     ↓        ↓
	//    Exp      Field
	//
	// lastStackは最後から二番目
	// Field->childをlastStack->childにする
	lastStack->next->child->child = lastStack->child;

	// call/ccがいた場所を空にする
	//
	// cont
	//  ↓
	// Stack -> Stack ->   Stack ->
	//  ↓        ↓          ↓
	// Env      UNDEFINED  (+ 1 UNDEFINED)
	Obj* exp = cont->child->next->child;
	exp->child = NULL;
	exp->type = UNDEFINED;


	// (call/cc PROCEDURE) を (PROCEDURE cont) に変換する
	// (call/cc PROCEDURE cont)
	now->next->next = cont;
	*getRegister() = NULL;
	// (PROCEDURE cont)
	getLastStack()->child->child = now->next;
	    
	return 0;
}

int apply_set_ex() {
	Obj* now = getLastStack()->child->child;
	Obj* key = nameResolution(getEnv(), now->next->data.name);
	if (key == NULL) {
		printError("Variable must be bound");
		return -1;
	}

	key->child = now->next->next;
	replaceStack(NULL);

	return 0;
}

int apply_function() {
	Obj* proc = getLastStack()->child->child;
	Obj* env = createEnv(proc->child->data.p_env);
	pushEnv(env);
	Obj* var = proc->child->next;
	Obj* arg = proc;
	while (var) {
		if (arg->next == NULL) {
			printError("wrong number of arguments.");
			return -1;
		}
		Obj *a = arg->next;
		arg->next = arg->next->next;
		a->next = NULL;
		registerSymbol(env, var->data.name, a);

		var = var->next;
	}
	if (arg->next) {
		printError("wrong number of arguments.");
		return -1;
	}

	Data data;
	data.name = NULL;
	Obj* pop_env = newObj(POP_ENV_BLOCK, data);
	replaceStack(pop_env);
	pop_env->child = proc->child->child;
	return 0;
}

int apply_continuation() {
	// (cont c) をRegisterに退避
	Obj* cont = *getRegister() = getLastStack()->child->child;
	
	// Stackを復帰
	getRoot()->child->next = cont->child->next;
	// Envを復帰
	getRoot()->child->child->child->next = cont->child->child;

	// 引数を代入
	//
	// Root
	//  ↓
	// Stack -> Stack -> Stack ->
	//  ↓        ↓        ↓
	//          ここに代入;
	cont->next->next = NULL;
	replaceStack(cont->next);
	proceedStack();

	return 0;
}

int eval() {
	while (1) {
		if (getLastStack() == NULL) return 0;
		if (getLastStack()->child) {
			switch (getLastStack()->child->type) {
				case UNDEFINED:
					printError("detect undefined object.");
					return -1;

				case FIELD:
					pushStack(getLastStack()->child->child);
					continue;

				case QUOTE:
				{
					Obj* next = getLastStack()->child->next;
					replaceStack(getLastStack()->child->child);
					getLastStack()->child = next;
					continue;
				}

				case EXPRESSION:
					pushStack(getLastStack()->child->child);
					continue;

				case SYMBOL: {
					Obj* key = nameResolution(getEnv(), getLastStack()->child->data.name);
					Obj* obj = NULL;
					if (key) {
						if (copyObj(getRegister(), key->child) == -1) return -1;
						obj = *getRegister();
						*getRegister() = NULL;
					}
					if (obj == NULL) {
						switch (getLastStack()->child->data.name[0]) { // 組み込み関数
							case '+': GENERATE_BUILT_IN("+")
							case '-': GENERATE_BUILT_IN("-")
							case '*': GENERATE_BUILT_IN("*")
							case '<': GENERATE_BUILT_IN("<")
							case '>': GENERATE_BUILT_IN(">")
							case 'c': GENERATE_BUILT_IN("call/cc")
								  GENERATE_BUILT_IN("car")
								  GENERATE_BUILT_IN("cdr")

							case 'd': {
								if (strcmp(getLastStack()->child->data.name, "define") == 0) {
									if (eval_define(&obj) == -1) return -1;
									break;
								}
							}

							case 'i': {
								if (strcmp(getLastStack()->child->data.name, "if") == 0) {
									if (eval_if(&obj) == -1) return -1;
									break;
								}
							}

							case 'l': {
								if (strcmp(getLastStack()->child->data.name, "lambda") == 0) {
									if (eval_lambda(&obj) == -1) return -1;
									break;
								}

								GENERATE_BUILT_IN("list")

								if (strcmp(getLastStack()->child->data.name, "let") == 0) {
									if (eval_let(&obj) == -1) return -1;
									break;
								}
							}

							case 's': {
								if (strcmp(getLastStack()->child->data.name, "set!") == 0) {
									if (eval_set_ex(&obj) == -1) return -1;
									break;
								}
							}
						}
					}
					if (obj == NULL) {
						char str[60];
						sprintf(str, "%s is unbound variable", getLastStack()->child->data.name);
						printError(str);
						return -1;
					}
					replaceStack(obj);
					proceedStack();
					continue;
				}

				case POP_ENV_BLOCK:
					pushStack(getLastStack()->child->child);
					continue;

				default:
					proceedStack();
					continue;
			}
		} else {	// 式の端まで来た
			if (getParentStack() == NULL) return 0;

			if (getParentStack()->child->type == FIELD) {
				popStack();
				return 0;
			}

			if (getParentStack()->child->type == EXPRESSION) {
				popStack();
				if (getLastStack()->child->child == NULL) {
					printError("empty parentheses was found.");
					return -1;
				}

				if (getLastStack()->child->child->type == BUILT_IN) {
					APPLY_BUILT_IN("+", apply_add())
					APPLY_BUILT_IN("-", apply_sub())
					APPLY_BUILT_IN("*", apply_mul())
					APPLY_BUILT_IN("<", apply_less_than());
					APPLY_BUILT_IN(">", apply_greater_than());
					APPLY_BUILT_IN("define-var", apply_define_var());
					APPLY_BUILT_IN("define-func", apply_define_func());
					APPLY_BUILT_IN("if", apply_if());
					APPLY_BUILT_IN("lambda", apply_lambda());
					APPLY_BUILT_IN("list", apply_list());
					APPLY_BUILT_IN("call/cc", apply_call_cc());
					APPLY_BUILT_IN("car", apply_car());
					APPLY_BUILT_IN("cdr", apply_cdr());
					APPLY_BUILT_IN("set!", apply_set_ex());
					proceedStack();
				}

				if (getLastStack()->child->child->type == PROCEDURE) {
					// tail-recursion
					if (getParentStack() != NULL && getParentStack()->child->type == POP_ENV_BLOCK) {// 自分の親がPOP_ENV_BLOCKか確認
						if (getLastStack()->child->next == NULL) {    // 自分が末尾か確認
							popStack();
							popEnv();
							replaceStack(getLastStack()->child->child);		// POP_ENV_BLOCKを外して末尾最適化
						}
					}
					if (apply_function() == -1) return -1;
					continue;
				}

				if (getLastStack()->child->child->type == CONTINUATION) {
					if (apply_continuation() == -1) return -1;
					continue;
				}

				printError("bad expression");
				return -1;
			}

			if (getParentStack()->child->type == POP_ENV_BLOCK) {
				popStack();
				popEnv();
				Obj* last = getLastStack()->child->child;
				if (last) {
					while (last->next) {
						last = last->next;
					}
				}
				replaceStack(last);
				proceedStack();
			}
		}
	}
}
