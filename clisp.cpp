#include <assert.h>
#include <cstddef>
#include <iostream>

#include "clisp.hpp"

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
        if (c.atomType == atom::string_i || c.atomType == atom::num_i) {
            return listIn;
        }
        return envIn->find(std::get<atom>(listIn.content).content);
    } else if (std::get<vector<list>>(listIn.content).size() == 0) {
        return emptyList();
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

    if (std::get<vector<list>>(listIn.content).size() == 1) {
        return eval(std::get<vector<list>>(listIn.content).at(0), envIn);
    }

    assert(li.at(0).listType == list::listType::element_l);
    string op = std::get<atom>(li.at(0).content).content;
    vector<list> args = {};
    for (size_t i = 1; i < li.size(); i++) {
        args.push_back(li.at(i));
    }

    if (op == "abs") {
        assert(args.at(0).listType == list::listType::element_l);
        string v = std::get<atom>(args.at(0).content).content;
        double abs = std::abs(std::atof(v.c_str()));
        return list { list::listType::element_l, atom { std::to_string(abs), atom::num_i }, true };
    } else if (op == "+") {
        list lhs = eval(args.at(0), envIn);
        list rhs = eval(args.at(1), envIn);
        assert(rhs.listType == list::listType::element_l && lhs.listType == list::listType::element_l);
        double lhsD = std::atof(std::get<atom>(lhs.content).content.c_str());
        double rhsD = std::atof(std::get<atom>(rhs.content).content.c_str());
        double res = lhsD + rhsD;
        return list { list::listType::element_l, atom { std::to_string(res), atom::num_i }, true };
    } else if (op == "-") {
        list lhs = eval(args.at(0), envIn);
        list rhs = eval(args.at(1), envIn);
        assert(rhs.listType == list::listType::element_l && lhs.listType == list::listType::element_l);
        double lhsD = std::atof(std::get<atom>(lhs.content).content.c_str());
        double rhsD = std::atof(std::get<atom>(rhs.content).content.c_str());
        double res = lhsD - rhsD;
        return list { list::listType::element_l, atom { std::to_string(res), atom::num_i }, true };
    } else if (op == "*") {
        list lhs = eval(args.at(0), envIn);
        list rhs = eval(args.at(1), envIn);
        assert(rhs.listType == list::listType::element_l && lhs.listType == list::listType::element_l);
        double lhsD = std::atof(std::get<atom>(lhs.content).content.c_str());
        double rhsD = std::atof(std::get<atom>(rhs.content).content.c_str());
        double res = lhsD * rhsD;
        return list { list::listType::element_l, atom { std::to_string(res), atom::num_i }, true };
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
    } else if (op == "define") {
        assert(args.size() >= 2
            && args.at(0).listType == list::listType::element_l
            && std::get<atom>(args.at(0).content).atomType == atom::word_i);
        list res = eval(args.at(1), envIn);
        envIn->content.insert({ std::get<atom>(args.at(0).content).content, res });
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
