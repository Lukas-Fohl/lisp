#include <assert.h>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "clisp.hpp"

int main(int argc, char* argv[])
{
    if (argc > 2) {
        std::cout << "Too many arguments. Usage: clisp <file> or clisp -c" << std::endl;
        return 1;
    }

    if (argc < 2) {
        std::cout << "No file provided. Usage: clisp <file> or clisp -c" << std::endl;
        return 1;
    }

    std::string arg = argv[1];

    if (arg == "-c") {
        std::string buffer;
        while (true) {
            std::cout << (buffer.empty() ? "clisp> " : "... ");
            std::string lineIn;
            if (!std::getline(std::cin, lineIn)) {
                break;
            }
            buffer += lineIn;
            if (buffer.empty())
                continue;

            int parens = 0;
            for (char c : buffer) {
                if (parens < 0)
                    giveUp("what are you doing?");
                if (c == '(')
                    parens++;
                if (c == ')')
                    parens--;
            }

            if (parens == 0) {
                try {
                    auto result = readList("(" + buffer + ")");
                    buffer.clear();
                    std::vector<list> exprList = std::get<1>(result);
                    if (exprList.size() < 1) {
                        break;
                    }
                    eval(exprList.at(0));
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    buffer.clear();
                }
            }
        }
    } else {
        evalFile(arg);
    }

    return 0;
}

/*
reading rules:
 - all in "" as one
 - number as number
 - new array when ()
 - {' , `} as single element
 - else just as word

COOL things i need to build CURR
 - [x] env
 - [x] Prcedure (lambda)
    - [x] class
    - [x] print
    - [x] everything in eval
        - [x] if lambda return procedure
        - [x] if not found eval -> check for lambda
            - lambda call:
                - first element of list can be:
                    [x] list
                        check if first is lambda
                        -> eval to proc
                        -> call
                    [x] word
                        -> check if return is procedure
                        -> call
                    [x] procedure
                        -> call
        - [x] on call insert new env -> in class

 - macro -> do the macro stuff
    - if "macro" is found put like function into macro
    - before eval anything:
        - search over all elements:
            - if macro found -> replace + eval if contained "," or args for macro
        - eval resulting list
    - macro rules:
        - format: (defmacro (args...) (body-expression))
        - body rules:
            - continue with expression
            - everything led by "," gets evaluated

TODO:
 - add some common function
   -> move into speratre files/funcs
   -> if, cond, etc.
 - everything as double
 - [x] imports
 - cli
 */
