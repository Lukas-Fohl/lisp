#include <assert.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

using std::endl, std::cout, std::string, std::variant, std::vector, std::tuple, std::map;

typedef struct {
    string content;
    enum {
        string_i,
        num_i,
        quote_i,
        comma_i,
        backComma_i,
        word_i,
        atomType_size,
    } atomType;
} atom;

typedef struct list {
    enum class listType {
        element_l,
        list_l,
        empty_l
    } listType;

    std::variant<
        atom,
        vector<list>>
        content;
} list;
// typedef vector<list*> list_t;

list emptyList()
{
    return list { list::listType::empty_l, atom { "'", atom::quote_i } };
}

void giveUp(string reason, unsigned exitCode = 1)
{
    std::cerr << reason << endl;
    std::exit(exitCode);
}

class env {
public:
    map<string, list> content;
    env* outer;

    env()
    {
        this->content = {};
        this->outer = NULL;
    }
    env(map<string, list> contentIn, env* outerIn = NULL)
        : content(contentIn)
        , outer(outerIn)
    {
    }
    env(vector<string> namesIn, vector<list> elementsIn, env* outerIn = NULL)
        : outer(outerIn)
    {
        assert(namesIn.size() == elementsIn.size());
        for (size_t i = 0; i < namesIn.size(); i++) {
            this->content.insert({ namesIn.at(i), elementsIn.at(i) });
        }
    }
    list find(string target)
    {
        auto location = this->content.find(target);
        if (location != this->content.end()) {
            return location->second;
        } else if (this->outer != NULL) {
            return outer->find(target);
        } else {
            giveUp("cannot find: " + target);
        }
        return emptyList();
    }
};

env defaultEnv()
{
    return env();
}
env globalEnv = defaultEnv();

bool isWhiteSpace(char c)
{
    return c == ' ' || c == '\n' || c == '\t';
}

bool isNumber(char c)
{
    return (unsigned)c >= 48 && (unsigned)c <= 57;
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
            temp.push_back(list { list::listType::element_l, atom { "'", atom::quote_i } });
            break;
        case ',':
            temp.push_back(list { list::listType::element_l, atom { ",", atom::comma_i } });
            break;
        case '`':
            temp.push_back(list { list::listType::element_l, atom { "`", atom::backComma_i } });
            break;
        case '\"':
            // todo escape
            if (!isString) {
                temp.push_back(list { list::listType::element_l, atom { "", atom::string_i } });
            }
            isString = !isString;
            break;
        case '(': {
            tuple<unsigned, vector<list>> getNew = readList(input.substr(inputIdx + 1));
            inputIdx += std::get<0>(getNew) + 1;
            // cout << std::get<0>(getNew) << endl;
            temp.push_back(list { list::listType::list_l, std::get<1>(getNew) });
            break;
        }
        case ')':
            return { inputIdx, temp };
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
                    temp.push_back(list { list::listType::element_l, atom { string(1, input.at(inputIdx)), atom::num_i } });
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
                temp.push_back(list { list::listType::element_l, atom { string(1, input.at(inputIdx)), atom::num_i } });
                break;
            }

            if (isWord) {
                std::get<atom>(temp.at(temp.size() - 1).content).content += input.at(inputIdx);
            } else {
                temp.push_back(list { list::listType::element_l, atom { string(1, input.at(inputIdx)), atom::word_i } });
                isWord = true;
                isNum = false;
                isString = false;
            }
        }
    }

    return { inputIdx, temp };
}

void printList(std::vector<list> listIn)
{
    for (unsigned i = 0; i < listIn.size(); i++) {
        if(listIn.at(i).listType == list::listType::empty_l) {
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
        } else {
            cout << "(";
            printList(std::get<std::vector<list>>(listIn.at(i).content));
            cout << ")" << ((i == listIn.size() - 1) ? "" : ", ");
        }
    }
}
list eval(vector<list> listIn, env* envIn = &globalEnv);
list eval(list listIn, env* envIn = &globalEnv)
{
    assert(listIn.listType != list::listType::empty_l);
    if (listIn.listType == list::listType::element_l) {
        atom c = std::get<atom>(listIn.content);
        if (c.atomType == atom::string_i || c.atomType == atom::num_i) {
            return listIn;
        }
        return envIn->find(std::get<atom>(listIn.content).content);
        // cout << std::get<atom>(listIn->content).content << endl;
    } else if (std::get<vector<list>>(listIn.content).size() == 0) {
        return emptyList();
    }

    vector<list> li = std::get<vector<list>>(listIn.content);
    if (li.at(0).listType == list::listType::list_l) {
        giveUp("cannot eval list");
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
        string v = std::get<atom>(args.at(0).content).content;
        if (v.find(".") == v.npos) {
            int abs = std::abs(std::atoi(v.c_str()));
            return list { list::listType::element_l, atom { std::to_string(abs), atom::num_i } };
        } else {
            double abs = std::abs(std::atof(v.c_str()));
            return list { list::listType::element_l, atom { std::to_string(abs), atom::num_i } };
        }
    } else if (op == "car") {
        return std::get<vector<list>>(args.at(0).content).at(0);
        // return eval(std::get<vector<list>>(args.at(0).content).at(0),envIn);
    } else if (op == "cdr") {
        assert(args.size() > 0 && args.at(0).listType == list::listType::list_l);
        vector<list> rest = {};
        vector<list> argFirstList = std::get<vector<list>>(args.at(0).content);
        // vector<list> argFirstList = {eval(std::get<vector<list>>(args.at(0).content),envIn)};
        for (size_t i = 1; i < argFirstList.size(); i++) {
            rest.push_back(argFirstList.at(i));
        }
        return list { list::listType::list_l, rest };
    } else if (op == "define") {
        // check next is quote i.e. ', some
        // eval next
        // insert / set
        assert(args.size() >= 3
            && args.at(0).listType == list::listType::element_l
            && std::get<atom>(args.at(0).content).atomType == atom::quote_i
            && args.at(1).listType == list::listType::element_l
            && std::get<atom>(args.at(1).content).atomType == atom::word_i);
        list res = eval(args.at(2), envIn);
        envIn->content.insert({ std::get<atom>(args.at(1).content).content, res });
    } else if (op == "list") {
        vector<list> rest = {};
        for (auto l : args) {
            rest.push_back(eval(l, envIn));
        }
        return list { list::listType::list_l, rest };
    } else {
        giveUp("WTF is " + op);
    }

    // do the shit
    return emptyList();
}
list eval(vector<list> listIn, env* envIn) // still default
{
    return eval(list { list::listType::list_l, listIn }, envIn);
}

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

COOL things i need to build
 - [x] env
 - Prcedure (lambda)
    - class
    - print
    - everything in eval
        - if lambda return procedure
        - if not found eval -> check for lambda
        - on call insert new env -> in class

 - do
    - take multiple thing and just eval in order

 - macro -> do the macro stuff

 - add some common function
 - everything as double

 - [ ] function notion
 - [ ] eval function
 */
