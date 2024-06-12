#define FTK_MATH_EXPR_STARTUP
#include "Expr.h"

#include <stdexcept>

namespace FTK::Math
{
    Expr::Expr(const std::string &rawExpr) : rawExpr(rawExpr), calc(rawExpr.c_str())
    {
    }

    Expr::Expr(const Expr &other) : Expr(other.rawExpr)
    {
    }

    Expr::~Expr()
    {
    }

    std::string Expr::getRawExpr() const
    {
        return rawExpr;
    }

    bool Expr::evalBool(const Context &context) const
    {
        return calc.eval(context).asBool();
    }

    double Expr::evalDouble(const Context &context) const
    {
        return calc.eval(context).asDouble();
    }

    Expression::Expression(const std::string &rawExpr) : Expr(rawExpr)
    {
    }

    Expression::Expression(const Expression &other) : Expression(other.rawExpr)
    {
    }

    Expression::~Expression()
    {
    }

    double Expression::operator()(const Context &context) const
    {
        return eval(context);
    }

    double Expression::eval(const Context &context) const
    {
        return evalDouble(context);
    }

    Condition::Condition(const std::string &rawExpr) : Expr(rawExpr)
    {
    }

    Condition::Condition(const Condition &other) : Condition(other.rawExpr)
    {
    }

    Condition::~Condition()
    {
    }

    bool Condition::operator()(const Context &context) const
    {
        return eval(context);
    }

    bool Condition::eval(const Context &context) const
    {
        return evalBool(context);
    }

} // namespace FTK::Math
