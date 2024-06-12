#ifndef FTK_INVENTORY_H
#define FTK_INVENTORY_H

#include <unordered_set>

#include "Item.h"

namespace FTK
{
    class Inventory
    {
    public:
        Inventory();
        Inventory(const Inventory &other);

        int getGold() const;
        std::multiset<ItemData> getItems() const;
        std::multiset<Equipment> getEquipments() const;

        int getItemAmount(const std::string &itemID);

        void setGold(int newValue);
        void increaseGold(int increment = 1);
        void decreaseGold(int decrement = 1);

        void addItem(const std::string &id, int amount = 1);
        void removeItem(const std::string &id, int amount = 1);

        void addEquipment(const std::optional<Equipment> &equipment);
        std::optional<Equipment> removeEquipment(const uuids::uuid &uuid);

    private:
        Inventory(int gold, const std::multiset<ItemData> &items, const std::multiset<Equipment> &equipments);

        int gold;
        std::multiset<ItemData> items;
        std::multiset<Equipment> equipments;

        friend nlohmann::adl_serializer<Inventory>;
    };
} // namespace FTK

#endif // FTK_INVENTORY_H
