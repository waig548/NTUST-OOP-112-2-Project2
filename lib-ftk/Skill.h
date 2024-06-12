#ifndef FTK_SKILL_H
#define FTK_SKILL_H

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "defs.h"
#include "Expr.h"
#include "Action.h"

namespace FTK
{

    struct ActiveSkill
    {
        const std::string id;
        const std::string name;
        const size_t diceRolls;
        const Math::Expression rollChanceExpr;
        const bool allowPartial;
        const SkillType skillType;
        const TargetType targetType;
        const TargetScope targetScope;
        const std::vector<std::shared_ptr<Action>> actions;
        const size_t baseCooldown;
        const std::string description;

        Math::Context getContext(const std::string &key, const Math::Context &base = Math::Context::default_global());
    };

    enum class PassiveTriggerType
    {
        None,
        BeforeAttack,
        BeforeAttacked,
        OnDamaged,
        AfterDamaged,
        AfterAttack,
        AfterAttacked
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(PassiveTriggerType,
                                 {{PassiveTriggerType::None, "none"},
                                  {PassiveTriggerType::BeforeAttack, "before_attack"},
                                  {PassiveTriggerType::BeforeAttacked, "before_attacked"},
                                  {PassiveTriggerType::OnDamaged, "on_damaged"},
                                  {PassiveTriggerType::AfterDamaged, "after_damaged"},
                                  {PassiveTriggerType::AfterAttack, "after_attack"},
                                  {PassiveTriggerType::AfterAttacked, "after_attacked"}})

    struct PassiveSkill
    {
        const std::string id;
        const std::string name;
        const PassiveTriggerType triggerType;
        const bool requireActiveSkill;
        const std::vector<std::shared_ptr<Action>> actions;
        const Math::Condition condition;
        const size_t baseCooldown;
        const std::string description;
    };
} // namespace FTK

#endif // FTK_SKILL_H
