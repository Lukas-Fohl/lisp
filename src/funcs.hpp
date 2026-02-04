#pragma once

#include "clisp.hpp"
#include <map>

using std::map, std::vector;

typedef list (*defaultFunc)(vector<list>, env*);

// arithmetic functions
list abs(vector<list> args, env* envIn);

list plus(vector<list> args, env* envIn);

list minus(vector<list> args, env* envIn);

list mult(vector<list> args, env* envIn);

list div(vector<list> args, env* envIn);

list sqrt(vector<list> args, env* envIn);

list power(vector<list> args, env* envIn);

// control flow functions
list if_func(vector<list> args, env* envIn);

list cond_func(vector<list> args, env* envIn);

// list operations
list cons(vector<list> args, env* envIn);

list append(vector<list> args, env* envIn);

list car(vector<list> args, env* envIn);

list cdr(vector<list> args, env* envIn);

list nth(vector<list> args, env* envIn);

list list_func(vector<list> args, env* envIn);

map<string, defaultFunc> arithmeticFuncMap = {
    { "abs", abs },
    { "+", plus },
    { "-", minus },
    { "*", mult },
    { "/", div },
    { "sqrt", sqrt },
    { "power", power },
};

map<string, defaultFunc> controlFlowFuncMap = {
    { "if", if_func },
    { "cond", cond_func },
};

map<string, defaultFunc> listOperationsFuncMap = {
    { "cons", cons },
    { "append", append },
    { "car", car },
    { "cdr", cdr },
    { "nth", nth },
    { "list", list_func },
};
