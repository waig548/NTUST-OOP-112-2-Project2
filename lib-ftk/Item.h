#ifndef FTK_ITEM_H
#define FTK_ITEM_H

#include <nlohmann/json.hpp>

#include "defs.h"
#include "Modifier.h"

namespace FTK
{
    class Action;
    class Item
    {
    };

    struct ItemTemplate
    {
        const std::string id;
        const std::string name;
        const bool availableInCombat;
        const std::vector<std::shared_ptr<Action>> actionsOnUse;
        const int shopValue;
        const std::string description;
    };

    struct ItemData
    {
        const std::string id;
        mutable int amount;

        bool expired() const;

        friend bool operator<(const ItemData &a, const ItemData &b);
    };

    enum class EquipmentType
    {
        None,
        Weapon,
        Armor,
        Accessory
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(EquipmentType,
                                 {{EquipmentType::None, "none"},
                                  {EquipmentType::Weapon, "weapon"},
                                  {EquipmentType::Armor, "armor"},
                                  {EquipmentType::Accessory, "accessory"}})

    class Equipment
    {
    public:
        Equipment(const Equipment &other);

        std::map<std::string, std::set<uuids::uuid>> getAttachedModifiers() const;
        std::map<std::string, std::multiset<Modifier>> getModifierData() const;

        void markModAttached(const std::string &path, const uuids::uuid &uuid);
        void markModDetached(const std::string &path, const uuids::uuid &uuid);

        const uuids::uuid uuid;
        const std::string id;

        friend bool operator<(const Equipment &a, const Equipment &b);

    private:
        Equipment(const std::string &id, const std::map<std::string, std::multiset<Modifier>> &modifierData);
        Equipment(const uuids::uuid &uuid, const std::string &id, const std::map<std::string, std::set<uuids::uuid>> &attachedModifiers, const std::map<std::string, std::multiset<Modifier>> &modifierData);

        std::map<std::string, std::set<uuids::uuid>> attachedModifiers;
        std::map<std::string, std::multiset<Modifier>> modifierData;

        friend class Inventory;
        friend struct EquipmentTemplate;

        friend nlohmann::adl_serializer<Equipment>;
    };

    struct EquipmentTemplate
    {
        const std::string id;
        const std::string name;
        const EquipmentType equipmentType;
        const int weaponDiceRolls;
        const DamageType weaponDamageType;
        const std::vector<ModifierTemplate> modifiers;
        const std::vector<std::string> skills;
        const int shopValue;
        const std::string description;

        Equipment build() const;
    };
} // namespace FTK

#endif // FTK_ITEM_H
