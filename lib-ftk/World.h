#ifndef FTK_WORLD_H
#define FTK_WORLD_H

#include <vector>
#include <string>
#include <map>
#include <memory>

#include "Vec.h"
#include "Rect.h"
#include "Entity.h"

namespace FTK
{
    class World
    {
    public:
        World(const Vec2i &dimension);
        World(const World &other);

        std::vector<std::shared_ptr<Player>> getPlayers() const;
        std::vector<std::shared_ptr<Entity>> getEntities() const;
        std::vector<std::shared_ptr<RectEntity>> getRectEntities() const;

        template <class Func>
        std::shared_ptr<Player> getPlayerBy(Func predicate) const
        {
            if (auto it = std::find_if(players.begin(), players.end(), predicate); it != players.end())
                return *it;
            return nullptr;
        }

        std::shared_ptr<Player> getPlayerByUUID(const uuids::uuid &uuid) const;
        std::shared_ptr<Player> getPlayerByName(const std::string &name) const;
        std::shared_ptr<Player> getPlayerByIndex(size_t idx) const;

        std::vector<std::shared_ptr<Player>> getPlayersAt(int x, int y) const;
        std::vector<std::shared_ptr<Player>> getPlayersAt(const Vec2i &pos) const;

        template <class Func>
        std::shared_ptr<Entity> getEntityBy(Func predicate) const
        {
            if (auto it = std::find_if(entities.begin(), entities.end(), predicate); it != entities.end())
                return *it;
            return nullptr;
        }

        std::shared_ptr<Entity> getEntityByUUID(const uuids::uuid &uuid) const;

        std::vector<std::shared_ptr<Entity>> getEntitiesAt(int x, int y) const;
        std::vector<std::shared_ptr<Entity>> getEntitiesAt(const Vec2i &pos) const;

        std::shared_ptr<Rect> getRectAt(int x, int y) const;
        std::shared_ptr<Rect> getRectAt(const Vec2i &pos) const;

        std::shared_ptr<RectEntity> getRectEntityAt(int x, int y) const;
        std::shared_ptr<RectEntity> getRectEntityAt(const Vec2i &pos) const;

        bool inBound(int x, int y) const;
        bool inBound(const Vec2i &v) const;

        void addEntity(const std::shared_ptr<Entity> &e);
        void removeEntity(const uuids::uuid &entityUUID);

        void addPlayer(const std::shared_ptr<Player> &ep);
        void removePlayer(const uuids::uuid &playerUUID);

        void addRectEntityAt(const std::shared_ptr<RectEntity> &rectEntity, int x, int y);
        void addRectEntityAt(const std::shared_ptr<RectEntity> &rectEntity, const Vec2i &pos);
        void removeRectEntityAt(int x, int y);
        void removeRectEntityAt(const Vec2i &pos);

    private:
        World(const Vec2i &dimension, const std::vector<std::shared_ptr<Rect>> &rects, const std::vector<std::shared_ptr<Entity>> &entities, const std::vector<std::shared_ptr<Player>> &players);

    public:
        Vec2i dimension;
        std::vector<std::shared_ptr<Rect>> rects;
        std::vector<std::shared_ptr<Entity>> entities;
        std::vector<std::shared_ptr<Player>> players;

        friend nlohmann::adl_serializer<World>;
    };
} // namespace FTK

#endif // FTK_WORLD_H
