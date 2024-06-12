#include "World.h"

#include "utils.h"
namespace FTK
{
    World::World(const Vec2i &dimension) : World(dimension, {}, {}, {})
    {
        for (int i = 0; i < dimension.getY(); i++)
            for (int j = 0; j < dimension.getX(); j++)
                rects.push_back(std::make_shared<Rect>("rect:path", 0));
    }

    World::World(const World &other) : World(other.dimension, other.rects, other.entities, other.players)
    {
    }

    std::vector<std::shared_ptr<Player>> World::getPlayers() const
    {
        return players;
    }

    std::vector<std::shared_ptr<Entity>> World::getEntities() const
    {
        return entities;
    }

    std::vector<std::shared_ptr<RectEntity>> World::getRectEntities() const
    {
        return map<std::shared_ptr<RectEntity>>(filter(rects, [](std::shared_ptr<Rect> r)
                                                       { return r->hasRectEntity(); }),
                                                [](std::shared_ptr<Rect> r)
                                                { return r->getRectEntity(); });
    }

    std::shared_ptr<Player> World::getPlayerByUUID(const uuids::uuid &uuid) const
    {
        return getPlayerBy([uuid](auto ep)
                           { return ep->uuid == uuid; });
    }

    std::shared_ptr<Player> World::getPlayerByName(const std::string &name) const
    {
        return getPlayerBy([name](auto ep)
                           { return ep->name == name; });
    }

    std::shared_ptr<Player> World::getPlayerByIndex(size_t idx) const
    {
        if (idx >= players.size())
            return nullptr;
        return players[idx];
    }

    std::vector<std::shared_ptr<Player>> World::getPlayersAt(int x, int y) const
    {
        return getPlayersAt({x, y});
    }

    std::vector<std::shared_ptr<Player>> World::getPlayersAt(const Vec2i &pos) const
    {
        return filter(players, [pos](std::shared_ptr<Player> ep)
                      { return ep->getPos() == pos; });
    }

    std::shared_ptr<Entity> World::getEntityByUUID(const uuids::uuid &uuid) const
    {
        return getEntityBy([uuid](auto ent)
                           { return ent->uuid == uuid; });
    }

    std::vector<std::shared_ptr<Entity>> World::getEntitiesAt(int x, int y) const
    {
        return getEntitiesAt({x, y});
    }

    std::vector<std::shared_ptr<Entity>> World::getEntitiesAt(const Vec2i &pos) const
    {
        return filter(entities, [pos](std::shared_ptr<Entity> e)
                      { return e->getPos() == pos; });
    }

    std::shared_ptr<Rect> World::getRectAt(int x, int y) const
    {
        if (!inBound(x, y))
            return nullptr;
        return rects[y * dimension.getX() + x];
    }

    std::shared_ptr<Rect> World::getRectAt(const Vec2i &pos) const
    {
        return getRectAt(pos.getX(), pos.getY());
    }

    std::shared_ptr<RectEntity> World::getRectEntityAt(int x, int y) const
    {
        if (auto rect = getRectAt(x, y); rect)
            return rect->getRectEntity();
        return nullptr;
    }

    std::shared_ptr<RectEntity> World::getRectEntityAt(const Vec2i &pos) const
    {
        return getRectEntityAt(pos.getX(), pos.getY());
    }

    bool World::inBound(int x, int y) const
    {
        return x >= 0 && x < dimension.getX() && y >= 0 && y < dimension.getY();
    }

    bool World::inBound(const Vec2i &v) const
    {
        return inBound(v.getX(), v.getY());
    }

    void World::addEntity(const std::shared_ptr<Entity> &e)
    {
        entities.push_back(e);
    }

    void World::removeEntity(const uuids::uuid &entityUUID)
    {
        if (auto it = std::find_if(entities.begin(), entities.end(), [entityUUID](auto e)
                                   { return e->uuid == entityUUID; });
            it != entities.end())
        {
            entities.erase(it);
        }
    }

    void World::addPlayer(const std::shared_ptr<Player> &ep)
    {
        players.push_back(ep);
    }

    void World::removePlayer(const uuids::uuid &playerUUID)
    {
        if (auto it = std::find_if(players.begin(), players.end(), [playerUUID](auto e)
                                   { return e->uuid == playerUUID; });
            it != players.end())
        {
            players.erase(it);
        }
    }

    void World::addRectEntityAt(const std::shared_ptr<RectEntity> &re, int x, int y)
    {
        if (auto rect = getRectAt(x, y); rect)
            rect->attachRectEntity(re);
    }

    void World::addRectEntityAt(const std::shared_ptr<RectEntity> &re, const Vec2i &pos)
    {
        addRectEntityAt(re, pos.getX(), pos.getY());
    }

    void World::removeRectEntityAt(int x, int y)
    {
        if (auto rect = getRectAt(x, y); rect)
            rect->removeRectEntity();
    }

    void World::removeRectEntityAt(const Vec2i &pos)
    {
        removeRectEntityAt(pos.getX(), pos.getY());
    }

    World::World(const Vec2i &dimension, const std::vector<std::shared_ptr<Rect>> &rects, const std::vector<std::shared_ptr<Entity>> &entities, const std::vector<std::shared_ptr<Player>> &players)
        : dimension(dimension), rects(rects), entities(entities), players(players)
    {
        for (auto ep : this->players)
        {
            auto ctr = ep->getPos();
            for (auto offset : FTK::ManhattanDistanceOffsets)
            {
                auto target = ctr + offset;
                if (inBound(target))
                    getRectAt(target)->markVisible();
            }
        }
    }

} // namespace FTK
