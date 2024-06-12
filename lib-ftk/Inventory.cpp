#include "Inventory.h"

#include "Registry.h"

namespace FTK
{
    Inventory::Inventory() : Inventory(600, {}, {})
    {
        // for (auto eq : *MainRegistry::getInstance()->equipmentTemplates)
        // {
        //     equipments.insert(eq.build());
        //     equipments.insert(eq.build());
        // }

        // for(auto item: *MainRegistry::getInstance()->itemTemplates)
        // {
        //     items.insert(ItemData{item.id, 99});
        // }
    }

    Inventory::Inventory(const Inventory &other) : Inventory(other.gold, other.items, other.equipments)
    {
    }

    int Inventory::getGold() const
    {
        return gold;
    }

    std::multiset<ItemData> Inventory::getItems() const
    {
        return items;
    }

    std::multiset<Equipment> Inventory::getEquipments() const
    {
        return equipments;
    }

    int Inventory::getItemAmount(const std::string &itemID)
    {
        for (auto i : items)
            if (i.id == itemID)
                return i.amount;
        return 0;
    }

    void Inventory::setGold(int newValue)
    {
        gold = std::max(0, newValue);
    }

    void Inventory::increaseGold(int increment)
    {
        setGold(gold + increment);
    }

    void Inventory::decreaseGold(int decrement)
    {
        setGold(gold - decrement);
    }

    void Inventory::addItem(const std::string &id, int amount)
    {
        if (amount <= 0)
            return;
        if (auto it = std::find_if(items.begin(), items.end(), [id](auto item)
                                   { return item.id == id; });
            it != items.end())
            it->amount += amount;
        else
            items.insert({id, amount});
    }

    void Inventory::removeItem(const std::string &id, int amount)
    {
        if (amount <= 0)
            return;
        if (auto it = std::find_if(items.begin(), items.end(), [id](auto item)
                                   { return item.id == id; });
            it != items.end())
        {
            it->amount -= amount;
            if (it->expired())
                items.erase(it);
        }
    }

    void Inventory::addEquipment(const std::optional<Equipment> &equipment)
    {
        if (equipment)
        {
            auto tmp = *equipment;
            tmp.attachedModifiers.clear();
            equipments.insert(tmp);
        }
    }

    std::optional<Equipment> Inventory::removeEquipment(const uuids::uuid &uuid)
    {
        if (auto it = std::find_if(equipments.begin(), equipments.end(), [uuid](auto equipment)
                                   { return equipment.uuid == uuid; });
            it != equipments.end())
        {
            auto res = *it;
            equipments.erase(it);
            return res;
        }
        return std::nullopt;
    }

    Inventory::Inventory(int gold, const std::multiset<ItemData> &items, const std::multiset<Equipment> &equipments) : gold(gold), items(items), equipments(equipments)
    {
    }

} // namespace FTK
