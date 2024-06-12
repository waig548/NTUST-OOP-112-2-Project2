#include "Rect.h"

#include "Registry.h"
#include "GameManager.h"
namespace FTK
{
    RectEntity::RectEntity(const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos) : RectEntity(uuids::uuid_system_generator{}(), id, name, type, pos)
    {
    }

    RectEntity::RectEntity(const RectEntity &other) : RectEntity(other.id, other.name, other.type, other.pos)
    {
    }

    void RectEntity::onInteract(const std::shared_ptr<Player> &ep)
    {
    }

    RectEntity::RectEntity(const uuids::uuid &uuid, const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos)
        : uuid(uuid), id(id), name(name), type(type), pos(pos)
    {
    }

    std::string RectEntity::getSerialType() const
    {
        return "rect_entity";
    }

    ShopRectEntity::ShopRectEntity(const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos, const std::shared_ptr<Inventory> inventory) : ShopRectEntity(uuids::uuid_system_generator{}(), id, name, type, pos, inventory)
    {
    }

    ShopRectEntity::ShopRectEntity(const ShopRectEntity &other) : ShopRectEntity(other.uuid, other.id, other.name, other.type, other.pos, other.inventory)
    {
    }

    void ShopRectEntity::onInteract(const std::shared_ptr<Player> &ep)
    {
    }

    std::shared_ptr<Inventory> ShopRectEntity::getInventory() const
    {
        return inventory;
    }

    void ShopRectEntity::buyItem(const std::string &id, int price, int amount)
    {
        auto inv = GameManager::getInstance()->getInventory();
        if (inv->getGold() < price * amount)
            return;
        inv->decreaseGold(price * amount);
        inventory->removeItem(id, amount);
        inv->addItem(id, amount);
    }

    void ShopRectEntity::buyEquipment(const uuids::uuid &uuid, int price)
    {
        auto inv = GameManager::getInstance()->getInventory();
        if (inv->getGold() < price)
            return;
        inv->decreaseGold(price);
        inv->addEquipment(inventory->removeEquipment(uuid));
    }

    ShopRectEntity::ShopRectEntity(const uuids::uuid &uuid, const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos, const std::shared_ptr<Inventory> inventory)
        : RectEntity(uuid, id, name, type, pos), inventory(inventory)
    {
        if (id == "re.shop:demo_shop" && inventory->getEquipments().empty() && inventory->getItems().empty())
        {
            auto equipList = MainRegistry::getInstance()->equipmentTemplates;
            auto itemList = MainRegistry::getInstance()->itemTemplates;
            for (auto e : *equipList)
            {
                this->inventory->addEquipment(e.build());
                this->inventory->addEquipment(e.build());
                this->inventory->addEquipment(e.build());
            }
            for (auto i : *itemList)
            {
                this->inventory->addItem(i.id, 999);
            }
        }
    }

    std::string ShopRectEntity::getSerialType() const
    {
        return "shop_rect_entity";
    }

    RestRectEntity::RestRectEntity(const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos, const std::vector<std::shared_ptr<Action>> &restActions, const Math::Condition &restCondition) : RestRectEntity(uuids::uuid_system_generator{}(), id, name, type, pos, restActions, restCondition)
    {
    }

    RestRectEntity::RestRectEntity(const RestRectEntity &other) : RestRectEntity(other.uuid, other.id, other.name, other.type, other.pos, other.restActions, other.restCondition)
    {
    }

    void RestRectEntity::onInteract(const std::shared_ptr<Player> &ep)
    {
    }

    RestRectEntity::RestRectEntity(const uuids::uuid &uuid, const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos, const std::vector<std::shared_ptr<Action>> &restActions, const Math::Condition &restCondition)
        : RectEntity(uuid, id, name, type, pos), restActions(restActions), restCondition(restCondition)
    {
    }

    std::string RestRectEntity::getSerialType() const
    {
        return "rest_rect_entity";
    }

    Rect::Rect(const std::string &id, int metadata) : Rect(id, metadata, false)
    {
    }

    Rect::Rect(const std::string &id, int metadata, bool visible) : id(id), metadata(metadata), visible(visible)
    {
    }

    Rect::Rect(const Rect &other) : Rect(other.id, other.metadata, other.visible)
    {
    }

    std::string Rect::getID() const
    {
        return id;
    }

    int Rect::getMetadata() const
    {
        return metadata;
    }

    bool Rect::isVisible() const
    {
        return visible;
    }

    std::shared_ptr<RectEntity> Rect::getRectEntity() const
    {
        return rectEntity;
    }

    void Rect::markVisible()
    {
        visible = true;
    }

    bool Rect::traversable() const
    {
        return !(id == "rect:rock");
    }

    bool Rect::hasRectEntity() const
    {
        return rectEntity.operator bool();
    }

    void Rect::attachRectEntity(const std::shared_ptr<RectEntity> &re)
    {
        if (rectEntity)
            throw std::exception("There exists a RectEntity on this Rect already.");
        rectEntity = re;
    }

    void Rect::removeRectEntity()
    {
        rectEntity = {};
    }

    void Rect::onPlayerEntered(std::shared_ptr<Player> ep)
    {
    }

} // namespace FTK
