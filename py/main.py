import math
import operator as op
import sys

Symbol = str
Number = int|float
Atom = int|Symbol
List = list
Exp = Atom|List

def atom(token):
    try: 
        return int(token)
    except ValueError:
        try:
            return float(token)
        except ValueError:
            return Symbol(token)

def strToList(tokensIn):
    token = tokensIn.pop(0)
    if token == "(":
        r = []
        while tokensIn[0] != ')':
            r.append(strToList(tokensIn))
        tokensIn.pop(0)
        return r
    elif token == ")":
        raise SyntaxError("unexpected ')'")
    else:
        return atom(token)

class Env(dict):
    def __init__(self, params=(), args=(), outer=None):
        self.update(zip(params, args))
        self.outer = outer
    def find(self, var):
        if var in self:
            return self
        elif self.outer != None:
            return self.outer.find(var)
        else:
            return {}
        # return self if var in self else self.outer.find(var)

class procedure():
    def __init__(self, param, body, env):
        self.param = param
        self.body = body
        self.env = env
    def __call__(self, *args):
        return eval(self.body, Env(self.param, args, self.env))

def defaultEnv()->Env:
    ret = Env()
    ret.update(vars(math))
    ret.update({
        '+':op.add, '-':op.sub, '*':op.mul, '/':op.truediv, 
        '>':op.gt, '<':op.lt, '>=':op.ge, '<=':op.le, '=':op.eq, 
        'abs':     abs,
        'append':  op.add,
        'car':     lambda x: x[0],
        'cdr':     lambda x: x[1:],
        'cons':    lambda x,y: [x] + y,
        'len':     len,
        'map':     map,
        'max':     max,
        'min':     min,
        'expt':    pow,
        'equal?':  op.eq,
        'length':  len,
        'list':    lambda *x: List(x),
        'not':     op.not_,
        'atom':    lambda x: isinstance(x, Atom),
        'null?':   lambda x: x == [], 
        'list?':   lambda x: isinstance(x, List),
        'eq?':     op.is_,
        'equal?':  op.eq,
        'number?': lambda x: isinstance(x, Number),
        'string?': lambda x: isinstance(x, str),
        'print':    print,
        'procedure?': callable,
        'round':   round,
        'symbol?': lambda x: isinstance(x, Symbol),
    })
    return ret

global_env = defaultEnv()

def parse(data: str):
    data = data.replace("\n","")
    data = data.replace("\t","")
    data = data.replace("\r","")
    data = data.replace("(","( ")
    data = data.replace(")"," )")
    return strToList(data.split())

def eval(x, env=global_env):
    if isinstance(x, Symbol):
        return env.find(x)[x]
    elif not isinstance(x, list):
        return x
    elif x == []:
        return None

    op, *args = x
    if op == "quote":
        return args[0]
    if op == "cond":
        idx = 0
        while idx < len(args)-1:
            if eval(args[idx], env):
                return eval(args[idx+1], env)
            else:
                idx += 2
    elif op == "if":
        cond, cons, alt = args
        if eval(cond, env):
            return eval(cons, env)
        else:
            return eval(alt, env)
    elif op == "define":
        var, exp = args
        env[var] = eval(exp, env)
    elif op == "import":
        main(args[0])
    elif op == "and":
        lhs, rhs = args
        return (eval(lhs, env) and eval(rhs, env))
    elif op == "or":
        lhs, rhs = args
        return (eval(lhs, env) or eval(rhs, env))
    elif op == "nth":
        num, varList = args
        varList = eval(varList, env)
        return varList[num]
    elif op == "error":
        print(args)
        exit()
    # elif op == "eval":
    #     return eval(args,env)
    elif op == "listev":
        # return args
        return list(map(lambda el: eval(el, env),args))
    elif op == "set":
        var, exp = args
        #force eval var var = eval(var, env)
        if isinstance(var, list):
            var = env.find(var[0])[var[0]]
        env.find(var)[var] = eval(exp, env)
    elif op == "empty?":
        # print(args[0])
        # print(eval(args[0], env))
        return len(eval(args[0], env)) == 0
    elif op == "lambda":
        params, body = args
        return procedure(params, body, env)
    else:
        proc = eval(op, env)
        vals = [eval(arg, env) for arg in args]
        return proc(*vals)
    return

def main(intputFile: str):
    with open(intputFile, "r") as file:
        data = file.read().split(",")
        for i in data:
            if i.strip().startswith(";") or i.strip() == "":
                continue
            # print(eval(parse(i)))
            eval(parse(i))
    return

if __name__ == "__main__":
    main(sys.argv[1])
