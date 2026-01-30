#include "clisp.hpp"
#include <cmath>
#include <map>

using std::map, std::vector;

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
