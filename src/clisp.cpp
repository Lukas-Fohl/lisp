#include <assert.h>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

#include "clisp.hpp"
#include "funcs.hpp"

list emptyList()
{
    return list { list::listType::empty_l, atom { "", atom::empty_i }, true };
}

void giveUp(string reason, unsigned exitCode) // default in header for exitcode
{
    std::cerr << reason << endl;
    std::exit(exitCode);
}

env defaultEnv()
{
    return env();
}

bool isWhiteSpace(char c)
{
    return c == ' ' || c == '\n' || c == '\t';
}

bool isNumber(char c)
{
    return (unsigned)c >= 48 && (unsigned)c <= 57;
}

vector<list> clean(vector<list> listIn)
{
    vector<list> ret = {};
    for (size_t i = 0; i < listIn.size(); i++) {
        if (listIn.at(i).listType == list::listType::element_l
            && std::get<atom>(listIn.at(i).content).atomType == atom::quote_i
            && std::get<atom>(listIn.at(i).content).content == "'"
            && i != listIn.size() - 1) {
            listIn.at(i + 1).shouldEval = false;
            ret.push_back(listIn.at(i + 1));
            i++;
        } else {
            ret.push_back(listIn.at(i));
        }
    }
    return ret;
}

tuple<unsigned, vector<list>> readList(string input)
{

    vector<list> temp = {};
    bool isString = false;
    bool isNum = false;
    bool isWord = false;
    unsigned inputIdx = 0;
    for (; inputIdx < input.size(); inputIdx++) {
        if (isString && input.at(inputIdx) != '\"') {
            std::get<atom>(temp.at(temp.size() - 1).content).content += string(1, input.at(inputIdx));
            continue;
        }

        switch (input.at(inputIdx)) {
        case '\'':
            temp.push_back(list { list::listType::element_l, atom { "'", atom::quote_i }, true });
            break;
        case ',':
            temp.push_back(list { list::listType::element_l, atom { ",", atom::comma_i }, true });
            break;
        case '`':
            temp.push_back(list { list::listType::element_l, atom { "`", atom::backComma_i }, true });
            break;
        case '\"':
            // todo escape
            if (!isString) {
                temp.push_back(list { list::listType::element_l, atom { "", atom::string_i }, true });
            }
            isString = !isString;
            break;
        case '(': {
            tuple<unsigned, vector<list>> getNew = readList(input.substr(inputIdx + 1));
            inputIdx += std::get<0>(getNew) + 1;
            // cout << std::get<0>(getNew) << endl;
            temp.push_back(list { list::listType::list_l, std::get<1>(getNew), true });
            break;
        }
        case ')':
            return { inputIdx, clean(temp) };
            break;
        default:
            if (isWhiteSpace(input.at(inputIdx))) {
                isNum = false;
                isWord = false;
                isString = false;
                break;
            }
            if (
                !isNum && !isWord && !isString && (input.at(inputIdx) == 't' || input.at(inputIdx) == 'f')
                && !std::isalnum(input.at(inputIdx + 1))) {
                temp.push_back(list { list::listType::element_l, atom { string(1, input.at(inputIdx)), atom::bool_i }, true });
                isNum = true;
                isString = false;
                isWord = false;
                break;
            }

            if (isNumber(input.at(inputIdx)) && !isString && !isWord) {
                if (isNum) {
                    std::get<atom>(temp.at(temp.size() - 1).content).content += input.at(inputIdx);
                } else {
                    temp.push_back(list { list::listType::element_l, atom { string(1, input.at(inputIdx)), atom::num_i }, true });
                    isNum = true;
                    isString = false;
                    isWord = false;
                }
                break;
            }

            if (isNum && !isString && !isWord && input.at(inputIdx) == '.') {
                if (isNumber(input.at(inputIdx + 1))) {
                    std::get<atom>(temp.at(temp.size() - 1).content).content += input.at(inputIdx);
                } else {
                    giveUp("why the dot?");
                }
                break;
            } else if (input.at(inputIdx) == '-' && !isNum && !isWord && !isString && isNumber(input.at(inputIdx + 1))) {
                isNum = true;
                temp.push_back(list { list::listType::element_l, atom { string(1, input.at(inputIdx)), atom::num_i }, true });
                break;
            }

            if (isWord) {
                std::get<atom>(temp.at(temp.size() - 1).content).content += input.at(inputIdx);
            } else {
                temp.push_back(list { list::listType::element_l, atom { string(1, input.at(inputIdx)), atom::word_i }, true });
                isWord = true;
                isNum = false;
                isString = false;
            }
        }
    }

    return { inputIdx, clean(temp) };
}

void printList(std::vector<list> listIn)
{
    for (unsigned i = 0; i < listIn.size(); i++) {
        if (listIn.at(i).listType == list::listType::empty_l) {
            cout << "no result";
            continue;
        }
        // assert(listIn.at(i).listType != list::listType::empty_l);
        if (listIn.at(i).listType == list::listType::element_l) {

            string content = std::get<atom>(listIn.at(i).content).content;
            if (std::get<atom>(listIn.at(i).content).atomType == atom::num_i) {
                cout << "~" << content << "~";
            } else if (std::get<atom>(listIn.at(i).content).atomType == atom::string_i) {
                cout << "\"" << content << "\"";
            } else if (std::get<atom>(listIn.at(i).content).atomType == atom::bool_i) {
                cout << ((content == TRUE_STRING) ? "True" : "False");
            } else {
                cout << content;
            }

            cout << ((i == listIn.size() - 1) ? "" : ", ");
        } else if (listIn.at(i).listType == list::listType::list_l) {
            cout << "(";
            printList(std::get<std::vector<list>>(listIn.at(i).content));
            cout << ")" << ((i == listIn.size() - 1) ? "" : ", ");
        } else if (listIn.at(i).listType == list::listType::procedure_l) {
            cout << "(lambda (";
            procedure p = std::get<std::vector<procedure>>(listIn.at(i).content).at(0);
            for (size_t j = 0; j < p.argsNames.size(); j++) {
                cout << p.argsNames.at(j) << ((j == p.argsNames.size() - 1) ? "" : ", ");
            }
            cout << ") ";
            printList({ p.func });
            cout << ")" << ((i == listIn.size() - 1) ? "" : ", ");
        }
    }
}

list eval(list listIn, env* envIn)
{
    if (!listIn.shouldEval) {
        listIn.shouldEval = true;
        return listIn;
    }

    assert(listIn.listType != list::listType::empty_l);
    if (listIn.listType == list::listType::element_l) {
        atom c = std::get<atom>(listIn.content);
        if (c.atomType == atom::string_i || c.atomType == atom::num_i || c.atomType == atom::bool_i) {
            return listIn;
        }
        list found = envIn->find(std::get<atom>(listIn.content).content);
        if (found.listType == list::listType::procedure_l && std::get<vector<procedure>>(found.content).at(0).argsNames.size() == 0) {
            return std::get<vector<procedure>>(found.content).at(0).call(envIn);
        }
        return found;
    } else if (std::get<vector<list>>(listIn.content).size() == 0) {
        return emptyList();
    } else if (std::get<vector<list>>(listIn.content).size() == 1) {
        return eval(std::get<vector<list>>(listIn.content).at(0), envIn);
    }

    vector<list> li = std::get<vector<list>>(listIn.content);
    if (li.at(0).listType == list::listType::list_l) {
        if (std::get<atom>(std::get<vector<list>>(li.at(0).content).at(0).content).content == "lambda") {
            list func = eval(std::get<vector<list>>(li.at(0).content));
            procedure proc = std::get<vector<procedure>>(func.content).at(0);
            for (size_t i = 1; i < li.size(); i++) {
                proc.argsValues.push_back(eval(li.at(i), envIn));
                // printList({li.at(i)});
                // cout << endl;
            }
            return proc.call(envIn);
        } else {
            giveUp("cannot eval list");
        }
    }

    // if (std::get<vector<list>>(listIn.content).size() == 1) {
    //     return eval(std::get<vector<list>>(listIn.content).at(0), envIn);
    // }

    assert(li.at(0).listType == list::listType::element_l);
    string op = std::get<atom>(li.at(0).content).content;
    vector<list> args = {};
    for (size_t i = 1; i < li.size(); i++) {
        args.push_back(li.at(i));
    }

    if (auto findFunc = arithmeticFuncMap.find(op); findFunc != arithmeticFuncMap.end()) {
        return findFunc->second(args, envIn);
    } else if (op == "=") {
        list lhs = eval(args.at(0), envIn);
        list rhs = eval(args.at(1), envIn);
        if (lhs.listType != rhs.listType || lhs.listType == list::listType::empty_l || rhs.listType == list::listType::empty_l) { // undefined is false
            return list { list::listType::element_l, atom { FALSE_STRING, atom::bool_i }, true };
        }
        switch (lhs.listType) {
        case list::listType::list_l:
            giveUp("TODO impl list comp");
            break;
        case list::listType::procedure_l:
            giveUp("TODO impl procedure comp");
            break;
        case list::listType::empty_l:
            return list { list::listType::element_l, atom { FALSE_STRING, atom::bool_i }, true };
            break;
        case list::listType::element_l:
            if (std::get<atom>(lhs.content).atomType == atom::num_i && std::get<atom>(lhs.content).atomType == atom::num_i) {
                list lhs = eval(args.at(0), envIn);
                list rhs = eval(args.at(1), envIn);
                assert(rhs.listType == list::listType::element_l && lhs.listType == list::listType::element_l);
                double lhsD = std::atof(std::get<atom>(lhs.content).content.c_str());
                double rhsD = std::atof(std::get<atom>(rhs.content).content.c_str());
                if (lhsD == rhsD) {
                    return list { list::listType::element_l, atom { TRUE_STRING, atom::bool_i }, true };
                } else {
                    return list { list::listType::element_l, atom { FALSE_STRING, atom::bool_i }, true };
                }
            }

            if (std::get<atom>(lhs.content).content == std::get<atom>(rhs.content).content
                && std::get<atom>(lhs.content).atomType == std::get<atom>(rhs.content).atomType) {
                return list { list::listType::element_l, atom { TRUE_STRING, atom::bool_i }, true };
            }
            break;
        }
        return list { list::listType::element_l, atom { FALSE_STRING, atom::bool_i }, true };
    } else if (op == "not" || op == "!") {
        list res = eval(args.at(0), envIn);
        assert(res.listType == list::listType::element_l && std::get<atom>(res.content).atomType == atom::bool_i);
        string boolString = TRUE_STRING;
        if (std::get<atom>(res.content).content == TRUE_STRING) {
            boolString = FALSE_STRING;
        }
        return list { list::listType::element_l, atom { boolString, atom::bool_i }, true };

    } else if (op == "assert") {
        list argCheck = eval(args.at(0), envIn);
        assert(argCheck.listType == list::listType::element_l
            && std::get<atom>(argCheck.content).atomType == atom::bool_i);
        if (std::get<atom>(argCheck.content).content == TRUE_STRING) {
            return emptyList();
        }
        list errorRes = eval(args.at(1), envIn);
        printList({ errorRes });
        cout << endl;
        abort();
    } else if (op == "cons") {
        list argLHS = eval(args.at(0), envIn);
        list argRHS = eval(args.at(1), envIn);
        assert(argLHS.listType == list::listType::list_l && argRHS.listType == list::listType::list_l);
        auto res = std::get<vector<list>>(argLHS.content);
        for (auto el : std::get<vector<list>>(argRHS.content)) {
            res.push_back(el);
        }
        return list { list::listType::list_l, vector<list> { res }, true };
    } else if (op == "car") {
        list argRes = eval(args.at(0), envIn);
        assert(argRes.listType == list::listType::list_l);
        return std::get<vector<list>>(argRes.content).at(0);
    } else if (op == "cdr") {
        list argRes = eval(args.at(0), envIn);
        assert(argRes.listType == list::listType::list_l);
        vector<list> rest = {};
        for (size_t i = 1; i < std::get<vector<list>>(argRes.content).size(); i++) {
            rest.push_back(std::get<vector<list>>(argRes.content).at(i));
        }
        return list { list::listType::list_l, rest, true };
    } else if (op == "nth") {
        list idxEl = eval(args.at(0), envIn);
        assert(idxEl.listType == list::listType::element_l && std::get<atom>(idxEl.content).atomType == atom::num_i);
        size_t idx = std::atoi(std::get<atom>(idxEl.content).content.c_str());
        list argRes = eval(args.at(1), envIn);
        assert(argRes.listType == list::listType::list_l);
        return std::get<vector<list>>(argRes.content).at(idx);
    } else if (op == "define") {
        assert(args.size() >= 2
            && args.at(0).listType == list::listType::element_l
            && std::get<atom>(args.at(0).content).atomType == atom::word_i);
        list res = eval(args.at(1), envIn);
        string name = std::get<atom>(args.at(0).content).content;
        auto procList = envIn->safeFind(name);
        if (procList) {
            envIn->content[name] = res;
        } else {
            envIn->content.insert({ name, res });
        }
    } else if (op == "list") {
        vector<list> rest = {};
        for (auto l : args) {
            rest.push_back(eval(l, envIn));
        }
        return list { list::listType::list_l, rest, true };
    } else if (op == "print") {
        printList({ eval(args.at(0), envIn) });
        cout << endl;
        return emptyList();
    } else if (op == "import") {
        for (list importElement : args) {
            assert(importElement.listType == list::listType::element_l && std::get<atom>(importElement.content).atomType == atom::string_i);
            evalFile(std::get<atom>(importElement.content).content);
        }
        return emptyList();
    } else if (op == "lambda") {
        vector<string> argsNames = {};
        assert(args.size() == 2);
        assert(args.at(0).listType == list::listType::list_l);
        vector<list> oneList = std::get<vector<list>>(args.at(0).content);
        for (size_t i = 0; i < oneList.size(); i++) {
            assert(oneList.at(i).listType == list::listType::element_l);
            assert(std::get<atom>(oneList.at(i).content).atomType == atom::word_i);
            argsNames.push_back(std::get<atom>(oneList.at(i).content).content);
        }
        return list { list::listType::procedure_l, vector<procedure> { procedure(argsNames, args.at(1)) }, true };
    } else {
        list procList = envIn->find(op);
        if (procList.listType == list::listType::procedure_l) {
            procedure proc = std::get<std::vector<procedure>>(procList.content).at(0);
            // Evaluate arguments
            vector<list> evaledArgs;
            for (auto& arg : args) {
                evaledArgs.push_back(eval(arg, envIn));
            }
            proc.argsValues = evaledArgs;
            return proc.call(envIn);
        } else if (procList.listType == list::listType::element_l) {
            vector<list> newList = { (procList) };
            newList.insert(newList.end(), args.begin(), args.end());
            return eval(newList, envIn);
        } else {
            giveUp("WTF is " + op);
        }
    }

    // do the shit
    return emptyList();
}
list eval(vector<list> listIn, env* envIn) // still default
{
    return eval(list { list::listType::list_l, listIn, true }, envIn);
}

void evalFile(string path)
{
    string fileName = path;
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
}
