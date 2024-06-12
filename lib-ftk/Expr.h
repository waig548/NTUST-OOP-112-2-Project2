#ifndef FTK_MATH_EXPR_H
#define FTK_MATH_EXPR_H

#include <any>
#include <map>
#include <optional>
#include <string>
#include <variant>

#include <shunting-yard.h>

#include "defs.h"

namespace FTK::Math
{
    using Context = cparse::TokenMap;

    class Expr
    {
    public:
        Expr(const std::string &rawExpr);
        Expr(const Expr &other);
        virtual ~Expr() = 0;

        std::string getRawExpr()const;

    protected:
        bool evalBool(const Context &context = {}) const;
        double evalDouble(const Context &context = {}) const;

        std::string rawExpr;
        cparse::calculator calc;
    };

    class Expression : public Expr
    {
    public:
        Expression(const std::string &rawExpr);
        Expression(const Expression &other);
        ~Expression() override;

        double operator()(const Context &context = Context::default_global()) const;

        double eval(const Context &context = Context::default_global()) const;

        friend nlohmann::adl_serializer<Expression>;
    };

    class Condition : public Expr
    {
    public:
        Condition(const std::string &rawExpr);
        Condition(const Condition &other);
        ~Condition() override;

        bool operator()(const Context &context = Context::default_global()) const;

        bool eval(const Context &context = Context::default_global()) const;

        friend nlohmann::adl_serializer<Condition>;
    };

} // namespace FTK::Math

#endif // FTK_MATH_EXPR_H

#ifdef FTK_MATH_EXPR_STARTUP

namespace cparse
{
#include <shunting-yard.h>
#include <builtin-features.inc>

    struct Startup
    {
        static packToken min(TokenMap scope)
        {
            return std::min(scope["a"].asDouble(), scope["b"].asDouble());
        }

        static packToken max(TokenMap scope)
        {
            return std::max(scope["a"].asDouble(), scope["b"].asDouble());
        }

        Startup()
        {
            cparse_startup();
            auto &global = TokenMap::default_global();
            global["min"] = CppFunction(&min, args_t{"a", "b"}, "min");
            global["max"] = CppFunction(&max, args_t{"a", "b"}, "max");
            global["DamageType"] = TokenMap();
            global["DamageType"]["Default"] = (int)FTK::DamageType::Default;
            global["DamageType"]["Physical"] = (int)FTK::DamageType::Physical;
            global["DamageType"]["Magical"] = (int)FTK::DamageType::Magical;
            global["DamageType"]["Heal"] = (int)FTK::DamageType::Heal;
            global["DamageType"]["True"] = (int)FTK::DamageType::True;
            global["TargetType"] = TokenMap();
            global["TargetType"]["None"] = (int)FTK::TargetType::None;
            global["TargetType"]["Self"] = (int)FTK::TargetType::Self;
            global["TargetType"]["Single"] = (int)FTK::TargetType::Single;
            global["TargetType"]["Multi"] = (int)FTK::TargetType::Multi;
            global["TargetType"]["Main"] = (int)FTK::TargetType::Main;
            global["TargetType"]["SplashExcludingMain"] = (int)FTK::TargetType::SplashExcludingMain;
            global["TargetType"]["Splash"] = (int)FTK::TargetType::Splash;
            global["SkillType"] = TokenMap();
            global["SkillType"]["None"] = (int)FTK::SkillType::None;
            global["SkillType"]["Attack"] = (int)FTK::SkillType::Attack;
            global["SkillType"]["Debuff"] = (int)FTK::SkillType::Debuff;
            global["SkillType"]["Buff"] = (int)FTK::SkillType::Buff;
            global["SkillType"]["Heal"] = (int)FTK::SkillType::Heal;
            global["SkillType"]["Flee"] = (int)FTK::SkillType::Flee;
            global["ActionType"] = TokenMap();
            global["ActionType"]["Nop"] = (int)FTK::ActionType::Nop;
            global["ActionType"]["Damage"] = (int)FTK::ActionType::Damage;
            global["ActionType"]["Heal"] = (int)FTK::ActionType::Heal;
            global["ActionType"]["Buff"] = (int)FTK::ActionType::Buff;
            global["ActionType"]["Debuff"] = (int)FTK::ActionType::Debuff;
            global["ActionType"]["Flee"] = (int)FTK::ActionType::Flee;
            global["ActionType"]["Destroy"] = (int)FTK::ActionType::Destroy;
            global["ActionType"]["AddModifier"] = (int)FTK::ActionType::AddModifier;
            global["ActionType"]["RemoveModifier"] = (int)FTK::ActionType::RemoveModifier;
            global["ActionType"]["ModifyStat"] = (int)FTK::ActionType::ModifyStat;
        }
    } ftk_expr_startup;

} // namespace cparse

#endif // FTK_MATH_EXPR_STARTUP