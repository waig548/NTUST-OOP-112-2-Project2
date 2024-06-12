#ifndef FTK_RECT_H
#define FTK_RECT_H

#include <memory>
#include <string>

#include <nlohmann/adl_serializer.hpp>

#include "Vec.h"
// #include "World.h"
#include "Entity.h"
#include "Item.h"
#include "Inventory.h"

namespace FTK
{
    enum class RectEntityType
    {
        None,
        Shop,
        Chest,
        Event,
        Rest
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(RectEntityType, {
                                                     {RectEntityType::None, "none"},
                                                     {RectEntityType::Shop, "shop"},
                                                     {RectEntityType::Chest, "chest"},
                                                     {RectEntityType::Event, "event"},
                                                     {RectEntityType::Rest, "rest"},
                                                 })

    class RectEntity
    {
    public:
        RectEntity(const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos);
        RectEntity(const RectEntity &other);

        virtual void onInteract(const std::shared_ptr<Player> &ep);

        const uuids::uuid uuid;
        const std::string id;
        const std::string name;
        const RectEntityType type;
        const Vec2i pos;

    protected:
        RectEntity(const uuids::uuid &uuid, const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos);

        virtual std::string getSerialType() const;
        friend nlohmann::adl_serializer<RectEntity>;
        friend nlohmann::adl_serializer<std::shared_ptr<RectEntity>>;
    };
    class ShopRectEntity : public RectEntity
    {
    public:
        ShopRectEntity(const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos, const std::shared_ptr<Inventory> inventory);
        ShopRectEntity(const ShopRectEntity &other);
        void onInteract(const std::shared_ptr<Player> &ep) override;

        std::shared_ptr<Inventory> getInventory() const;

        void buyItem(const std::string &id, int price, int amount = 1);
        void buyEquipment(const uuids::uuid &uuid, int price);

    private:
        ShopRectEntity(const uuids::uuid &uuid, const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos, const std::shared_ptr<Inventory> inventory);

        std::string getSerialType() const override;

        std::shared_ptr<Inventory> inventory;

        friend nlohmann::adl_serializer<ShopRectEntity>;
    };

    class RestRectEntity : public RectEntity
    {
    public:
        RestRectEntity(const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos, const std::vector<std::shared_ptr<Action>> &restActions, const Math::Condition &restCondition);
        RestRectEntity(const RestRectEntity &other);
        void onInteract(const std::shared_ptr<Player> &ep) override;

    private:
        RestRectEntity(const uuids::uuid &uuid, const std::string &id, const std::string &name, RectEntityType type, const Vec2i &pos, const std::vector<std::shared_ptr<Action>> &restActions, const Math::Condition &restCondition);

        std::string getSerialType() const override;

        std::vector<std::shared_ptr<Action>> restActions;
        Math::Condition restCondition;

        friend nlohmann::adl_serializer<RestRectEntity>;
    };

    class Rect
    {
    public:
        Rect(const std::string &id, int metadata);
        Rect(const std::string &id, int metadata, bool visible);
        Rect(const Rect &other);

        std::string getID() const;
        int getMetadata() const;
        bool isVisible() const;
        std::shared_ptr<RectEntity> getRectEntity() const;

        void markVisible();

        bool traversable() const;
        bool hasRectEntity() const;

        void attachRectEntity(const std::shared_ptr<RectEntity> &re);
        void removeRectEntity();

        void onPlayerEntered(std::shared_ptr<Player> ep);

    private:
        std::string id;
        int metadata;
        bool visible;
        std::shared_ptr<RectEntity> rectEntity;

        friend nlohmann::adl_serializer<Rect>;
    };
} // namespace FTK

#endif // FTK_RECT_H
