#include <assert.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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
        printList({ eval(a) });
        cout << endl;
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
 - [ ] Prcedure (lambda)
    - [x] class
    - [x] print
    - [ ] everything in eval
        - [ ] if lambda return procedure
        - [ ] if not found eval -> check for lambda
        - [ ] on call insert new env -> in class

 - do
    - take multiple thing and just eval in order

 - macro -> do the macro stuff

 - add some common function
 - everything as double

 - [ ] function notion
 - [ ] eval function
 */
