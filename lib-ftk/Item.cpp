#include "Item.h"

#include "utils.h"
#include "Action.h"

namespace FTK
{
    bool ItemData::expired() const
    {
        return amount <= 0;
    }

    Equipment::Equipment(const Equipment &other) : Equipment(other.uuid, other.id, other.attachedModifiers, other.modifierData)
    {
    }

    std::map<std::string, std::set<uuids::uuid>> Equipment::getAttachedModifiers() const
    {
        return attachedModifiers;
    }

    std::map<std::string, std::multiset<Modifier>> Equipment::getModifierData() const
    {
        return modifierData;
    }

    void Equipment::markModAttached(const std::string &path, const uuids::uuid &uuid)
    {
        attachedModifiers[path].insert(uuid);
    }

    void Equipment::markModDetached(const std::string &path, const uuids::uuid &uuid)
    {
        if(attachedModifiers.count(path))
        {
            attachedModifiers[path].erase(uuid);
            if(attachedModifiers.at(path).empty())
                attachedModifiers.erase(path);
        }
    }

    Equipment::Equipment(const std::string &id, const std::map<std::string, std::multiset<Modifier>> &modifierData) : Equipment(uuids::uuid_system_generator{}(), id, {}, modifierData)
    {
    }

    Equipment::Equipment(const uuids::uuid &uuid, const std::string &id, const std::map<std::string, std::set<uuids::uuid>> &attachedModifiers, const std::map<std::string, std::multiset<Modifier>> &modifierData) : uuid(uuid), id(id), attachedModifiers(attachedModifiers), modifierData(modifierData)
    {
    }

    Equipment EquipmentTemplate::build() const
    {
        std::map<std::string, std::multiset<Modifier>> modifierData;
        for (auto p : map<std::pair<std::string, Modifier>>(modifiers, [](ModifierTemplate modTemp)
                                                            { return modTemp.build(); }))
            modifierData[p.first].insert(p.second);
        return Equipment(id, modifierData);
    }

    bool operator<(const ItemData &a, const ItemData &b)
    {
        return a.id < b.id &&a.amount < b.amount;
    }

    bool operator<(const Equipment &a, const Equipment &b)
    {
        return a.id < b.id && a.uuid < b.uuid;
    }

} // namespace FTK