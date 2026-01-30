#pragma once

#include "clisp.hpp"
#include <map>

using std::map, std::vector;

typedef list (*defaultFunc)(vector<list>, env*);

list abs(vector<list> args, env* envIn);

list plus(vector<list> args, env* envIn);

list minus(vector<list> args, env* envIn);

list mult(vector<list> args, env* envIn);

list div(vector<list> args, env* envIn);

list sqrt(vector<list> args, env* envIn);

list power(vector<list> args, env* envIn);

map<string, defaultFunc> arithmeticFuncMap = {
    { "abs", abs },
    { "+", plus },
    { "-", minus },
    { "*", mult },
    { "/", div },
    { "sqrt", sqrt },
    { "power", power },
};
