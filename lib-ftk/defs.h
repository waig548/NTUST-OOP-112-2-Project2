#ifndef FTK_DEFS_H
#define FTK_DEFS_H

#include <map>
#include <vector>
#include <string>
#include <optional>

#include "Vec.h"

namespace FTK
{
    enum class DamageType
    {
        Default,
        Physical,
        Magical,
        Heal,
        True
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(DamageType,
                                 {{DamageType::Default, "default"},
                                  {DamageType::Physical, "physical"},
                                  {DamageType::Magical, "magical"},
                                  {DamageType::Heal, "heal"},
                                  {DamageType::True, "true"}})

    enum class TargetScope
    {
        Ally,
        Enemy,
        All
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TargetScope,
                                 {{TargetScope::Enemy, "enemy"},
                                  {TargetScope::Ally, "ally"},
                                  {TargetScope::All, "all"}})

    enum class TargetType
    {
        None,
        Self,
        Single,
        Multi,
        Main,
        SplashExcludingMain,
        Splash
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TargetType,
                                 {{TargetType::None, "none"},
                                  {TargetType::Self, "self"},
                                  {TargetType::Single, "single"},
                                  {TargetType::Multi, "multi"},
                                  {TargetType::Main, "main"},
                                  {TargetType::SplashExcludingMain, "splash_excluding_main"},
                                  {TargetType::Splash, "splash"}})

    enum class SkillType
    {
        None,
        Attack,
        Debuff,
        Buff,
        Heal,
        Flee
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(SkillType, {{SkillType::None, "none"},
                                             {SkillType::Attack, "attack"},
                                             {SkillType::Debuff, "debuff"},
                                             {SkillType::Buff, "buff"},
                                             {SkillType::Heal, "heal"},
                                             {SkillType::Flee, "flee"}})

    enum class ActionType
    {
        Nop,
        Damage,
        Heal,
        Buff,
        Debuff,
        Flee,
        Destroy,
        AddModifier,
        RemoveModifier,
        ModifyStat
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(ActionType,
                                 {{ActionType::Nop, "nop"},
                                  {ActionType::Damage, "damage"},
                                  {ActionType::Heal, "heal"},
                                  {ActionType::Buff, "buff"},
                                  {ActionType::Debuff, "debuff"},
                                  {ActionType::Flee, "flee"},
                                  {ActionType::Destroy, "destroy"},
                                  {ActionType::AddModifier, "add_modifier"},
                                  {ActionType::RemoveModifier, "remove_modifier"},
                                  {ActionType::ModifyStat, "modify_stat"}})

    struct Range
    {
        const double min;
        const double max;
    };

    struct AttributeDefinition
    {
        const std::string name;
        const Range range;
        const double defaultValue;
    };

    const std::vector<AttributeDefinition> GeneralAttributeDefs{
        {"max_hp", {0, 100}, 50},
        {"p_atk", {0, 100}, 10},
        {"p_def", {0, 100}, 5},
        {"m_atk", {0, 100}, 0},
        {"m_def", {0, 100}, 0},
        {"hit_rate", {0, 100}, 50},
        {"speed", {0, 100}, 50},
        {"dmg_taken", {0, DBL_MAX}, 1},
        {"dice_chance_mult", {0, DBL_MAX}, 1}};

    const std::vector<AttributeDefinition> PlayerAdditionalAttributeDefs{
        {"max_focus", {0, 100}, 3},
        {"ap_boost", {0, 100}, 0}};

    const auto ATTRIBUTE_RANGES = [](const std::string &key) -> std::optional<AttributeDefinition>
    {
        if (auto it = std::find_if(GeneralAttributeDefs.begin(), GeneralAttributeDefs.end(), [key](auto e)
                                   { return e.name == key; });
            it != GeneralAttributeDefs.end())
            return *it;
        if (auto it = std::find_if(PlayerAdditionalAttributeDefs.begin(), PlayerAdditionalAttributeDefs.end(), [key](auto e)
                                   { return e.name == key; });
            it != PlayerAdditionalAttributeDefs.end())
            return *it;
        return std::nullopt;
    };

    constexpr const std::array<Vec2i, 4> Directions{
        Vec2i(0, -1),
        Vec2i(1, 0),
        Vec2i(0, 1),
        Vec2i(-1, 0)};

    constexpr const std::array<Vec2i, 25> ManhattanDistanceOffsets{
        // center
        Vec2i(0, 0),
        // dist = 1
        Vec2i(0, -1),
        Vec2i(1, 0),
        Vec2i(0, 1),
        Vec2i(-1, 0),
        // dist = 2
        Vec2i(0, -2),
        Vec2i(1, -1),
        Vec2i(2, 0),
        Vec2i(1, 1),
        Vec2i(0, 2),
        Vec2i(-1, 1),
        Vec2i(-2, 0),
        Vec2i(-1, -1),
        // dist = 3
        Vec2i(0, -3),
        Vec2i(1, -2),
        Vec2i(2, -1),
        Vec2i(3, 0),
        Vec2i(2, 1),
        Vec2i(1, 2),
        Vec2i(0, 3),
        Vec2i(-1, 2),
        Vec2i(-2, 1),
        Vec2i(-3, 0),
        Vec2i(-2, -1),
        Vec2i(-1, -2)};

    const auto isAdjacent = [](const Vec2i &a, const Vec2i &b)
    {
        return std::find(Directions.begin(), Directions.end(), a - b) != Directions.end();
    };

} // namespace FTK

#endif // FTK_DEFS_H
