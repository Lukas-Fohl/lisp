#pragma once

#include <assert.h>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#define FALSE_STRING "f"
#define TRUE_STRING "t"

using std::endl, std::cout, std::string, std::variant, std::vector, std::tuple, std::map;

typedef struct {
    string content;
    enum {
        string_i,
        num_i,
        quote_i,
        comma_i,
        bool_i,
        backComma_i,
        word_i,
        empty_i,
        atomType_size,
    } atomType;
} atom;

class procedure;
class macro;

typedef struct list {
    enum class listType {
        element_l,
        list_l,
        empty_l,
        macro_l,
        procedure_l
    } listType;

    std::variant<
        atom,
        vector<list>,
        vector<macro>, // cuz fuck c++ and all its bullshit rules
        vector<procedure>> // cuz fuck c++ and all its bullshit rules
        content;
    bool shouldEval;
} list;

list emptyList();

void giveUp(string reason, unsigned exitCode = 1);

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
    std::optional<list> safeFind(string target)
    {
        auto location = this->content.find(target);
        if (location != this->content.end()) {
            return location->second;
        } else if (this->outer != NULL) {
            return outer->find(target);
        } else {
            return {};
        }
        return emptyList();
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

bool isWhiteSpace(char c);

bool isNumber(char c);

env defaultEnv();

tuple<unsigned, vector<list>> readList(string input);

void printList(std::vector<list> listIn);

inline env globalEnv = defaultEnv();

list eval(vector<list> listIn, env* envIn = &globalEnv);
list eval(list listIn, env* envIn = &globalEnv);

void evalFile(string path);

class macro {
public:
    vector<string> argsNames;
    list body;
    macro()
        : argsNames({})
        , body(emptyList())
    {
    }

    macro(vector<string> argsNamesIn, list funcIn)
        : argsNames(argsNamesIn)
        , body(funcIn)
    {
    }

    list replaceRecur(list listIn, map<string, list> macroReplaceList)
    {
        switch (listIn.listType) {
        case list::listType::empty_l:
        case list::listType::element_l:
            return listIn;
        case list::listType::list_l: {
            list returnList = list { list::listType::list_l, {}, true };
            vector<list> elementList = std::get<vector<list>>(listIn.content);
            for (size_t i = 0; i < elementList.size(); i++) {
                switch (listIn.listType) {
                case list::listType::empty_l:
                    break;
                case list::listType::element_l:
                    if (std::get<atom>(elementList.at(i).content).atomType == atom::comma_i) {
                        auto replaceSearch = macroReplaceList.find(std::get<atom>(elementList.at(i).content).content);
                        if (i != elementList.size() - 1 && replaceSearch != macroReplaceList.end()) {
                            i++;
                            std::get<vector<list>>(returnList.content).push_back(replaceSearch->second);
                        } else {
                            giveUp("cannot find in macro replace " + replaceSearch->first);
                        }
                    }
                    break;
                case list::listType::list_l:
                    std::get<vector<list>>(returnList.content).push_back(replaceRecur(elementList.at(i), macroReplaceList));
                    break;
                case list::listType::procedure_l:
                case list::listType::macro_l:
                    giveUp("New achievement: How Did We Get Here?", 1);
                    break;
                }
            }
            return returnList;
        }
        case list::listType::procedure_l:
        case list::listType::macro_l:
            giveUp("New achievement: How Did We Get Here?", 1);
            return listIn;
        }
    }

    list replace(vector<list> args)
    {
        assert(args.size() == this->argsNames.size());
        map<string, list> macroReplaceList = {};
        for (size_t i = 0; i < args.size(); i++) {
            macroReplaceList.insert({ this->argsNames.at(i), args.at(i) });
        }
        return replaceRecur(this->body, macroReplaceList);
    }
};

class procedure {
public:
    vector<string> argsNames;
    vector<list> argsValues;
    list func;
    procedure()
        : argsNames({})
        , argsValues({})
        , func(emptyList())
    {
    }

    procedure(vector<string> argsNamesIn, list funcIn)
        : argsNames(argsNamesIn)
        , argsValues({})
        , func(funcIn)
    {
    }
    procedure(vector<string> argsNamesIn, vector<list> argsValuesIn, list funcIn)
        : argsNames(argsNamesIn)
        , argsValues(argsValuesIn)
        , func(funcIn)
    {
    }

    list call(env* envIn)
    {
        assert(this->argsNames.size() == this->argsValues.size());
        env tempEnv = defaultEnv();
        for (size_t i = 0; i < this->argsNames.size(); i++) {
            tempEnv.content.insert({
                this->argsNames.at(i),
                this->argsValues.at(i),
            });
        }
        tempEnv.outer = envIn;
        return eval(this->func, &tempEnv);
    }
};
