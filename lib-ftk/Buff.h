
#ifndef FTK_BUFF_H
#define FTK_BUFF_H

#include <string>

#include <nlohmann/json.hpp>

#include "Modifier.h"

namespace FTK
{
    class Action;

    enum class EffectType
    {
        None,
        Modifier,
        Action,
        SkipTurn
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(EffectType,
                                 {{EffectType::None, "none"},
                                  {EffectType::Modifier, "modifier"},
                                  {EffectType::Action, "action"},
                                  {EffectType::SkipTurn, "skip_turn"}})

    class Buff
    {
    public:
        Buff(const Buff &other);

        int getTurns() const;
        std::map<std::string, std::set<uuids::uuid>> getAttachedModifiers() const;
        std::map<std::string, std::multiset<Modifier>> getModifierData() const;

        bool expired(int threshold = 0) const;

        void markAttached(const std::string &path, const uuids::uuid &uuid);

        void setTurns(int newValue) const;
        void decTurns(int decrement = 1) const;

        friend bool operator==(const Buff &a, const Buff &b);
        friend bool operator<(const Buff &a, const Buff &b);

        const uuids::uuid uuid;
        const std::string id;

    private:
        Buff(const std::string &id, int turns, const std::map<std::string, std::multiset<Modifier>> &modifierData);
        Buff(const uuids::uuid &uuid, const std::string &id, int turns, const std::map<std::string, std::set<uuids::uuid>> &attachedModifiers, const std::map<std::string, std::multiset<Modifier>> &modifierData);

        std::map<std::string, std::set<uuids::uuid>> attachedModifiers;
        std::map<std::string, std::multiset<Modifier>> modifierData;

        mutable int turns;

        friend struct BuffTemplate;

        friend nlohmann::adl_serializer<Buff>;
    };

    struct BuffTemplate
    {
    public:
        const std::string id;
        const std::string name;
        const EffectType effectType;
        const std::vector<ModifierTemplate> modifiers;
        const std::vector<std::shared_ptr<Action>> actions;
        const std::string description;

        Buff build(int turns);
    };

    struct BuffBuildData
    {
        const std::string id;
        const int turns;
    };
} // namespace FTK

#endif // FTK_BUFF_H
