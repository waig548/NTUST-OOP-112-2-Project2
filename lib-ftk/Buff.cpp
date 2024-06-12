#include "Buff.h"

#include "utils.h"
#include "Action.h"

namespace FTK
{
    Buff::Buff(const Buff &other) : Buff(other.uuid, other.id, other.turns, other.attachedModifiers, other.modifierData)
    {
    }

    int Buff::getTurns() const
    {
        return turns;
    }

    std::map<std::string, std::set<uuids::uuid>> Buff::getAttachedModifiers() const
    {
        return attachedModifiers;
    }

    std::map<std::string, std::multiset<Modifier>> Buff::getModifierData() const
    {
        return modifierData;
    }

    bool Buff::expired(int threshold) const
    {
        return turns <= threshold;
    }

    void Buff::markAttached(const std::string &path, const uuids::uuid &uuid)
    {
        attachedModifiers[path].insert(uuid);
    }

    void Buff::setTurns(int newValue) const
    {
        turns = newValue;
    }

    void Buff::decTurns(int decrement) const
    {
        turns -= decrement;
    }

    Buff::Buff(const std::string &id, int turns, const std::map<std::string, std::multiset<Modifier>> &modifierData) : Buff(uuids::uuid_system_generator{}(), id, turns, {}, modifierData)
    {
    }

    Buff::Buff(const uuids::uuid &uuid, const std::string &id, int turns, const std::map<std::string, std::set<uuids::uuid>> &attachedModifiers, const std::map<std::string, std::multiset<Modifier>> &modifierData) : uuid(uuid), id(id), turns(turns), attachedModifiers(attachedModifiers), modifierData(modifierData)
    {
    }

    Buff BuffTemplate::build(int turns)
    {
        std::map<std::string, std::multiset<Modifier>> modifierData;
        for (auto p : map<std::pair<std::string, Modifier>>(modifiers, [](ModifierTemplate modTemp)
                                                            { return modTemp.build(); }))
            modifierData[p.first].insert(p.second);
        return Buff(id, turns, modifierData);
    }

    bool operator==(const Buff &a, const Buff &b)
    {
        if (a.id != b.id)
            return false;
        if (a.attachedModifiers.size() != b.attachedModifiers.size())
            return false;
        if (a.modifierData.size() != b.modifierData.size())
            return false;
        for (auto p : a.attachedModifiers)
        {
            if (b.attachedModifiers.count(p.first))
                return false;
            if (p.second.size() != b.attachedModifiers.at(p.first).size())
                return false;
            auto aAtch = p.second;
            auto bAtch = b.attachedModifiers.at(p.first);
            auto aIt = aAtch.begin(), bIt = bAtch.begin();
            while (aIt != aAtch.end() && bIt != bAtch.end())
            {
                if (*aIt != *bIt)
                    return false;
                aIt++;
                bIt++;
            }
        }
        for (auto p : a.modifierData)
        {
            if (!b.modifierData.count(p.first))
                return false;
            if (p.second.size() != b.modifierData.at(p.first).size())
                return false;
            auto aMod = p.second;
            auto bMod = b.modifierData.at(p.first);
            auto aIt = aMod.begin(), bIt = bMod.begin();
            while (aIt != aMod.end() && bIt != bMod.end())
            {
                if (aIt->type != bIt->type)
                    return false;
                if (aIt->name != bIt->name)
                    return false;
                if (aIt->value != bIt->value)
                    return false;
                aIt++;
                bIt++;
            }
        }

        return true;
    }

    bool operator<(const Buff &a, const Buff &b)
    {
        if (a.id != b.id)
            return false;
        if (a.modifierData.size() != b.modifierData.size())
            return false;
        for (auto p : a.modifierData)
            if (!b.modifierData.count(p.first))
                return false;
        for (auto p : a.modifierData)
            if (p.second.size() != b.modifierData.at(p.first).size())
                return false;
        for (auto p : a.modifierData)
        {
            auto aMod = p.second;
            auto bMod = b.modifierData.at(p.first);
            auto aIt = aMod.begin(), bIt = bMod.begin();
            while (aIt != aMod.end() && bIt != bMod.end())
            {
                if (aIt->type != bIt->type)
                    return false;
                if (aIt->name != bIt->name)
                    return false;
                if (aIt->value < bIt->value)
                    return true;
                aIt++;
                bIt++;
            }
        }

        return false;
    }
} // namespace FTK
