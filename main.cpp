#include <assert.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "clisp.hpp"

int main(int argn, char** argc)
{
    if (argn > 2) {
        cout << "i don't need to know all of that" << endl;
    }

    string fileName = argc[1];
    string fileContent("");
    string temp("");
    std::ifstream fileInput(fileName);
    while (getline(fileInput, temp)) {
        size_t comment = temp.find("--");
        if (comment == temp.npos) {
            fileContent.append(temp + "\n");
        } else {
            fileContent.append(temp.substr(0, comment));
        }
    }
    fileInput.close();

    vector<list> exprList = std::get<1>(readList(fileContent));
    for (list a : exprList) {
        // printList({ a });
        eval(a);
        // printList({ eval(a) });
        // cout << endl;
    }

    // env myEnv = { { "test" }, { new list { list::listType::element_l, atom { "'", atom::quote_i } } } };
    // for(auto a: myEnv.content){
    //     std::cout << a.first << endl;
    //     printList({a.second});
    // }

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

 - add some common function
 - everything as double
 */
