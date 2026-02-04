#include "clisp.hpp"
#include <cmath>
#include <map>

using std::map, std::vector;

// arithmetic functions

list abs(vector<list> args, env* envIn)
{
    list arg = eval(args.at(0), envIn);
    assert(arg.listType == list::listType::element_l);
    string v = std::get<atom>(arg.content).content;
    double abs = std::abs(std::atof(v.c_str()));
    return list { list::listType::element_l, atom { std::to_string(abs), atom::num_i }, true };
}

list plus(vector<list> args, env* envIn)
{
    list lhs = eval(args.at(0), envIn);
    list rhs = eval(args.at(1), envIn);
    assert(rhs.listType == list::listType::element_l && lhs.listType == list::listType::element_l);
    double lhsD = std::atof(std::get<atom>(lhs.content).content.c_str());
    double rhsD = std::atof(std::get<atom>(rhs.content).content.c_str());
    double res = lhsD + rhsD;
    return list { list::listType::element_l, atom { std::to_string(res), atom::num_i }, true };
}

list minus(vector<list> args, env* envIn)
{
    list lhs = eval(args.at(0), envIn);
    list rhs = eval(args.at(1), envIn);
    assert(rhs.listType == list::listType::element_l && lhs.listType == list::listType::element_l);
    double lhsD = std::atof(std::get<atom>(lhs.content).content.c_str());
    double rhsD = std::atof(std::get<atom>(rhs.content).content.c_str());
    double res = lhsD - rhsD;
    return list { list::listType::element_l, atom { std::to_string(res), atom::num_i }, true };
}

list mult(vector<list> args, env* envIn)
{
    list lhs = eval(args.at(0), envIn);
    list rhs = eval(args.at(1), envIn);
    assert(rhs.listType == list::listType::element_l && lhs.listType == list::listType::element_l);
    double lhsD = std::atof(std::get<atom>(lhs.content).content.c_str());
    double rhsD = std::atof(std::get<atom>(rhs.content).content.c_str());
    double res = lhsD * rhsD;
    return list { list::listType::element_l, atom { std::to_string(res), atom::num_i }, true };
}

list div(vector<list> args, env* envIn)
{
    list lhs = eval(args.at(0), envIn);
    list rhs = eval(args.at(1), envIn);
    assert(rhs.listType == list::listType::element_l && lhs.listType == list::listType::element_l);
    double lhsD = std::atof(std::get<atom>(lhs.content).content.c_str());
    double rhsD = std::atof(std::get<atom>(rhs.content).content.c_str());
    assert(rhsD != 0.0f);
    double res = lhsD / rhsD;
    return list { list::listType::element_l, atom { std::to_string(res), atom::num_i }, true };
}

list sqrt(vector<list> args, env* envIn)
{
    list numArg = eval(args.at(0), envIn);
    assert(numArg.listType == list::listType::element_l);
    double numArgD = std::atof(std::get<atom>(numArg.content).content.c_str());
    assert(numArgD >= 0.0f);
    double res = std::sqrt(numArgD);
    return list { list::listType::element_l, atom { std::to_string(res), atom::num_i }, true };
}

list power(vector<list> args, env* envIn)
{
    list lhs = eval(args.at(0), envIn);
    list rhs = eval(args.at(1), envIn);
    assert(rhs.listType == list::listType::element_l && lhs.listType == list::listType::element_l);
    double lhsD = std::atof(std::get<atom>(lhs.content).content.c_str());
    double rhsD = std::atof(std::get<atom>(rhs.content).content.c_str());
    double res = pow(lhsD, rhsD);
    return list { list::listType::element_l, atom { std::to_string(res), atom::num_i }, true };
}

// control flow functions

list if_func(vector<list> args, env* envIn)
{
    assert(args.size() == 3);
    list cond = eval(args.at(0), envIn);
    assert(cond.listType == list::listType::element_l
        && std::get<atom>(cond.content).atomType == atom::bool_i);
    if (std::get<atom>(cond.content).content == TRUE_STRING) {
        return eval(args.at(1), envIn);
    } else {
        return eval(args.at(2), envIn);
    }
}

list cond_func(vector<list> args, env* envIn)
{
    assert(args.size() % 2 == 0);
    for (size_t i = 0; i < args.size(); i += 2) {
        list cond = eval(args.at(i), envIn);
        if (std::get<atom>(cond.content).content == TRUE_STRING) {
            return eval(args.at(i + 1), envIn);
        }
    }
    return emptyList();
}

// list operations

list cons(vector<list> args, env* envIn)
{
    list argLHS = eval(args.at(0), envIn);
    list argRHS = eval(args.at(1), envIn);
    assert(argLHS.listType == list::listType::list_l && argRHS.listType == list::listType::list_l);
    auto res = std::get<vector<list>>(argLHS.content);
    for (auto el : std::get<vector<list>>(argRHS.content)) {
        res.push_back(el);
    }
    return list { list::listType::list_l, vector<list> { res }, true };
}

list append(vector<list> args, env* envIn)
{
    list argElement = eval(args.at(0), envIn);
    list argList = eval(args.at(1), envIn);
    assert(argList.listType == list::listType::list_l
        && argElement.listType == list::listType::element_l);
    std::get<vector<list>>(argList.content).push_back(argElement);
    return list { list::listType::list_l, std::get<vector<list>>(argList.content), true };
}

list car(vector<list> args, env* envIn)
{
    list argRes = eval(args.at(0), envIn);
    assert(argRes.listType == list::listType::list_l);
    if(std::get<vector<list>>(args.at(0).content).size() == 0){
        return args.at(0);
    }
    return std::get<vector<list>>(argRes.content).at(0);
}

list cdr(vector<list> args, env* envIn)
{
    list argRes = eval(args.at(0), envIn);
    assert(argRes.listType == list::listType::list_l);
    vector<list> rest = {};
    for (size_t i = 1; i < std::get<vector<list>>(argRes.content).size(); i++) {
        rest.push_back(std::get<vector<list>>(argRes.content).at(i));
    }
    return list { list::listType::list_l, rest, true };
}

list nth(vector<list> args, env* envIn)
{
    list idxEl = eval(args.at(0), envIn);
    assert(idxEl.listType == list::listType::element_l && std::get<atom>(idxEl.content).atomType == atom::num_i);
    size_t idx = std::atoi(std::get<atom>(idxEl.content).content.c_str());
    list argRes = eval(args.at(1), envIn);
    assert(argRes.listType == list::listType::list_l);
    return std::get<vector<list>>(argRes.content).at(idx);
}

list list_func(vector<list> args, env* envIn)
{
    vector<list> rest = {};
    for (auto l : args) {
        rest.push_back(eval(l, envIn));
    }
    return list { list::listType::list_l, rest, true };
}
