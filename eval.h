#ifndef _EVAL_H_
#define _EVAL_H_

#include <stdio.h>
#include <string.h>

#include "printError.h"
#include "obj.h"
#include "gc.h"
#include "core.h"
#include "env.h"

int eval_define(Obj** obj);
int eval_if(Obj** obj);
int eval_lambda(Obj** obj);
int eval_let(Obj** obj);
int eval_call_cc(Obj** obj);
int eval_set_ex(Obj** obj);

int apply_add();
int apply_sub();
int apply_mul();
int apply_less_than();
int apply_greater_than();
int apply_list();
int apply_car();
int apply_cdr();

int apply_define_var();
int apply_define_func();
int apply_if();
int apply_lambda();
int apply_call_cc();
int apply_set_ex();

int apply_function();
int apply_continuation();

int eval();

#endif
