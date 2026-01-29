#pragma once

#include <assert.h>
#include <iostream>
#include <map>
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
        procedure_l
    } listType;

    std::variant<
        atom,
        vector<list>,
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
    vector<list> argsValues;
    list func;
    macro()
        : argsNames({})
        , argsValues({})
        , func(emptyList())
    {
    }

    macro(vector<string> argsNamesIn, list funcIn)
        : argsNames(argsNamesIn)
        , argsValues({})
        , func(funcIn)
    {
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
