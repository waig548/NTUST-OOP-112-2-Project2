#include "GameManager.h"

#include <fstream>

#include <effolkronium/random.hpp>

#include "utils.h"
#include "Dice.h"
#include "combat.h"
#include "Serializer.h"

namespace FTK
{
    std::shared_ptr<World> GameManager::getWorld() const
    {
        return world;
    }

    GameState GameManager::getGameState() const
    {
        return gameState;
    }

    size_t GameManager::getRoundNumber() const
    {
        return round;
    }

    std::vector<uuids::uuid> GameManager::getPlayerTurnOrder() const
    {
        return playerTurnOrder;
    }

    size_t GameManager::getCurrentPlayerIndex() const
    {
        return currentPlayerIndex;
    }

    uuids::uuid GameManager::getCurrentPlayerUUID() const
    {
        return playerTurnOrder[currentPlayerIndex];
    }

    std::shared_ptr<Player> GameManager::getCurrentPlayer() const
    {
        return world->getPlayerByUUID(getCurrentPlayerUUID());
    }

    ExploreState GameManager::getExploreState() const
    {
        return exploreState;
    }

    std::shared_ptr<Inventory> GameManager::getInventory() const
    {
        return inventory;
    }

    bool GameManager::shouldEndMoveState() const
    {
        return gameState == GameState::Explore && exploreState == ExploreState::Move && !(getCurrentPlayer()->getAP());
    }

    InteractableType GameManager::getInteractableType(const Vec2i &pos) const
    {
        if (!(interactionFlags & InteractionFlag_InteractedWithEnemy) && world->getEntitiesAt(pos).size())
            return InteractableType::Enemy;
        if (!(interactionFlags & InteractionFlag_InteractedWithRE) && world->getRectAt(pos)->hasRectEntity())
            return InteractableType::RE;
        return InteractableType::None;
    }

    InteractableType GameManager::getCurrentInteractableType() const
    {
        return getInteractableType(getCurrentPlayer()->getPos());
    }

    void GameManager::beginTurn()
    {
        if (exploreState == ExploreState::BeginTurn)
        {
            if (getCurrentPlayer()->isDead())
            {
                exploreState = ExploreState::EndTurn;
                endTurn();
            }
            else
                exploreState = ExploreState::RollAP;
        }
    }

    void GameManager::rollAP(const uuids::uuid uuid, int focusUsed)
    {
        if (exploreState == ExploreState::RollAP)
        {
            auto ep = world->getPlayerByUUID(uuid);
            focusUsed = std::min(focusUsed, ep->getAsInt("focus"));
            ep->dec("focus", focusUsed);
            int maxAP = ep->getAsInt("speed") / 10;
            ep->set("max_ap", maxAP);
            size_t rolled = Dice::rollUniformDices(maxAP, ep->getAPChance(), focusUsed);
            ep->set("ap", rolled);
            exploreState = ExploreState::Move;
            if (!rolled)
                exploreState = ExploreState::EndTurn;
        }
    }

    bool GameManager::movePlayer(const uuids::uuid &uuid, const Vec2i &newPos, bool retreat, bool ignoreAdjacent)
    {
        if (exploreState != ExploreState::Move)
            return false;
        if (!isPosTraversable(newPos))
            return false;
        auto ep = world->getPlayerByUUID(uuid);

        if (ep->getPos() == newPos)
            return true;

        if (retreat)
        {
            ep->setPos(ep->getPrevPos(), retreat);
            interactionFlags = InteractionFlag_None;
        }
        else
        {
            if (gameState == GameState::Teleport)
            {
                if (inventory->getItemAmount("item:teleport_scroll") <= 0)
                    return false;
                if (world->getEntitiesAt(newPos).size())
                    return false;
                if (world->getRectEntityAt(newPos))
                    return false;
                inventory->removeItem("item:teleport_scroll");
                endTeleport();
            }
            else
            {
                if (!ep->getAP())
                    return false;
                if (!ignoreAdjacent && !isAdjacent(newPos, ep->getPos()))
                    return false;
                ep->dec("ap");
            }
            ep->setPos(newPos);
            interactionFlags = InteractionFlag_None;

            for (auto off : ManhattanDistanceOffsets)
            {
                auto tgt = newPos + off;
                if (world->inBound(tgt))
                    world->getRectAt(tgt)->markVisible();
            }

            if (getInteractableType(newPos) != InteractableType::None)
            {
                gameState = GameState::Interact;
            }
        }
        if (shouldEndMoveState())
            exploreState = ExploreState::EndTurn;

        return true;
    }

    bool GameManager::retreatPlayer(const uuids::uuid &uuid)
    {
        return movePlayer(uuid, world->getPlayerByUUID(uuid)->getPrevPos(), true, true);
    }

    void GameManager::endTurn()
    {
        if (exploreState == ExploreState::EndTurn)
        {
            currentPlayerIndex++;
            if (currentPlayerIndex >= playerTurnOrder.size())
                exploreState = ExploreState::EndRound;
            else
                exploreState = ExploreState::BeginTurn;
        }
    }

    void GameManager::rollAP(int focusUsed)
    {
        rollAP(getCurrentPlayerUUID(), focusUsed);
    }

    bool GameManager::movePlayer(const Vec2i &newPos, bool retreat, bool ignoreAdjacent)
    {
        return movePlayer(getCurrentPlayerUUID(), newPos, retreat, ignoreAdjacent);
    }

    bool GameManager::retreatPlayer()
    {
        return retreatPlayer(getCurrentPlayerUUID());
    }

    void GameManager::beginRound()
    {
        if (exploreState == ExploreState::BeginRound)
        {
            round++;
            currentPlayerIndex = 0;
            exploreState = ExploreState::BeginTurn;
        }
    }

    void GameManager::endRound()
    {
        if (exploreState == ExploreState::EndRound)
        {
            exploreState = ExploreState::BeginRound;
        }
    }

    void GameManager::enterShop()
    {
        if (gameState == GameState::Interact)
        {
            gameState = GameState::Shop;
        }
    }

    void GameManager::exitShop()
    {
        if (gameState == GameState::Shop)
        {
            gameState = GameState::Interact;
            markInteractionDone();
        }
    }

    void GameManager::beginTeleport()
    {
        gameState = GameState::Teleport;
    }

    void GameManager::endTeleport()
    {
        gameState = GameState::Interact;
        markInteractionDone();
    }

    void GameManager::useItem(const std::string &itemID)
    {
        auto itemData = MainRegistry::getInstance()->itemTemplates->get(itemID);
        if (itemID == "item:teleport_scroll")
        {
            beginTeleport();
            return;
        }
        ActionContext ctx;
        ctx.mathContext = getCurrentPlayer()->getMathContext("target", getCurrentPlayer()->getMathContext("self", ctx.mathContext));
        for (auto act : itemData.actionsOnUse)
        {
            act->apply(getCurrentPlayer(), getCurrentPlayer(), ctx);
        }
        inventory->removeItem(itemID);
    }

    void GameManager::triggerBattle(const uuids::uuid &triggerer, const Vec2i &pos, bool ambush, bool ambushFailed)
    {
        std::vector<uuids::uuid> playersToBattle;
        std::vector<uuids::uuid> enemiesToBattle;
        playersToBattle.push_back(triggerer);

        [this, &playersToBattle, pos]()
        {
            for (auto offset : ManhattanDistanceOffsets)
            {
                if (auto eps = world->getPlayersAt(pos + offset); eps.size())
                {
                    for (auto ep : eps)
                    {
                        if (!ep->isPlayer())
                            continue;
                        if (std::find(playersToBattle.begin(), playersToBattle.end(), ep->uuid) != playersToBattle.end())
                            continue;
                        while (playersToBattle.size() > 3)
                            playersToBattle.pop_back();
                        if (playersToBattle.size() == 3)
                            return;
                        playersToBattle.push_back(ep->uuid);
                    }
                }
            }
        }();

        [this, &enemiesToBattle, pos, ambush]()
        {
            for (auto offset : ManhattanDistanceOffsets)
            {
                if (auto ents = world->getEntitiesAt(pos + offset); ents.size())
                {
                    for (auto ent : ents)
                    {
                        if (!ent->isEnemy())
                            continue;
                        if (std::find(enemiesToBattle.begin(), enemiesToBattle.end(), ent->uuid) != enemiesToBattle.end())
                            continue;
                        while ((ambush && enemiesToBattle.size() > 1) || enemiesToBattle.size() > 3)
                            enemiesToBattle.pop_back();
                        if ((ambush && enemiesToBattle.size() == 1) || enemiesToBattle.size() == 3)
                            return;
                        enemiesToBattle.push_back(ent->uuid);
                    }
                }
            }
        }();

        gameState = GameState::Battle;
        CombatSystem::getInstance()->beginBattle(
            map<std::shared_ptr<Player>>(playersToBattle, [this](uuids::uuid uuid)
                                         { return getWorld()->getPlayerByUUID(uuid); }),
            map<std::shared_ptr<Enemy>>(enemiesToBattle, [this](uuids::uuid uuid)
                                        { return std::static_pointer_cast<Enemy>(getWorld()->getEntityByUUID(uuid)); }),
            ambushFailed);
    }

    void GameManager::triggerBattle(const Vec2i &pos, bool ambush, bool ambushFailed)
    {
        triggerBattle(getCurrentPlayerUUID(), pos, ambush, ambushFailed);
    }

    void GameManager::prepAPRoll()
    {
        if (exploreState == ExploreState::RollAP)
        {
            auto ep = world->getPlayerByUUID(getCurrentPlayerUUID());
            ep->set("max_ap", ep->getAsInt("speed") / 10);
        }
    }

    void GameManager::markAPRolled(size_t rolledAmount)
    {
        if (exploreState == ExploreState::RollAP)
        {
            auto ep = world->getPlayerByUUID(getCurrentPlayerUUID());
            ep->set("ap", rolledAmount);
            if (rolledAmount > 0)
                exploreState = ExploreState::Move;
            else
                exploreState = ExploreState::EndTurn;
        }
    }

    void GameManager::markInteractionDone(bool all)
    {
        if (gameState == GameState::Interact || gameState == GameState::Battle)
        {
            if (getExploreState() == ExploreState::EndTurn)
            {
                gameState = GameState::Explore;
                endTurn();
                return;
            }
            if (all)
                interactionFlags = ULLONG_MAX;
            switch (getCurrentInteractableType())
            {
            case InteractableType::Enemy:
                interactionFlags |= InteractionFlag_InteractedWithEnemy;
                break;
            case InteractableType::RE:
                interactionFlags |= InteractionFlag_InteractedWithRE;
                break;
            default:
                break;
            }
            if (getCurrentInteractableType() == InteractableType::None)
                gameState = GameState::Explore;
        }
    }

    void GameManager::markEntityDead(const uuids::uuid &entityUUID)
    {
        world->removeEntity(entityUUID);
    }

    void GameManager::markPlayerDead(const uuids::uuid &playerUUID)
    {
        if (getCurrentPlayerUUID() == playerUUID)
            exploreState = ExploreState::EndTurn;
    }

    void GameManager::saveMap(const std::string &path)
    {
        if (!std::filesystem::exists(path))
        {
            std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        }
        std::ofstream ofs(path);
        nlohmann::ordered_json j;
        j["world"] = world;
        j["game_state"] = gameState;
        j["round"] = round;
        j["turn_order"] = playerTurnOrder;
        j["current_player_index"] = currentPlayerIndex;
        j["explore_state"] = exploreState;
        j["interaction_flags"] = interactionFlags;
        j["inventory"] = inventory;
        j["combat_state"] = CombatSystem::getInstance()->saveState();
        std::stringstream ss;
        effolkronium::random_static::serialize(ss);
        j["random_state"] = ss.str();
        ofs << std::setw(4) << j;
    }

    void GameManager::saveMap(const char *path)
    {
        saveMap(std::string(path));
    }

    void GameManager::loadMap(const std::string &path)
    {
        reset();
        std::ifstream ifs(path);
        nlohmann::json j;
        ifs >> j;
        world = j["world"].get<std::shared_ptr<World>>();

        if (j.contains("game_state"))
        {
            gameState = j["game_state"].get<GameState>();
            round = j["round"].get<size_t>();
            playerTurnOrder = j["turn_order"].get<std::vector<uuids::uuid>>();
            currentPlayerIndex = j["current_player_index"].get<size_t>();
            exploreState = j["explore_state"].get<ExploreState>();
            interactionFlags = j["interaction_flags"].get<InteractionFlags>();
            inventory = j["inventory"].get<std::shared_ptr<Inventory>>();
            CombatSystem::getInstance()->retoreState(j["combat_state"]);
            std::stringstream ss(j["random_state"].get<std::string>());
            effolkronium::random_static::deserialize(ss);
        }
        else
        {
            for (auto e : world->entities)
            {
                e->initBuffs();
                e->initEquipments();
            }
            for (auto e : world->players)
            {
                e->initBuffs();
                e->initEquipments();
            }
        }
        if (!inventory)
            inventory = std::make_shared<Inventory>();
        restore(gameState == GameState::None);
    }

    void GameManager::loadMap(const char *path)
    {
        loadMap(std::string(path));
    }

    void GameManager::initGame()
    {
        auto eps = world->getPlayers();
        std::stable_sort(eps.begin(), eps.end(), [](auto p1, auto p2)
                         { return p1->uuid < p2->uuid; });
        std::stable_sort(eps.begin(), eps.end(), [](auto p1, auto p2)
                         { return p1->get("max_hp") > p2->get("max_hp"); });
        std::stable_sort(eps.begin(), eps.end(), [](auto p1, auto p2)
                         { return (p1->get("p_def") + p1->get("m_def")) > (p2->get("p_def") + p2->get("m_def")); });
        std::stable_sort(eps.begin(), eps.end(), [](auto p1, auto p2)
                         { return (p1->get("p_atk") + p1->get("m_atk")) > (p2->get("p_atk") + p2->get("m_atk")); });
        std::stable_sort(eps.begin(), eps.end(), [](auto p1, auto p2)
                         { return p1->get("speed") > p2->get("speed"); });
        playerTurnOrder = map<uuids::uuid>(eps, [](auto ep)
                                           { return ep->uuid; });
        effolkronium::random_static::reseed();
        gameState = GameState::Explore;
        exploreState = ExploreState::BeginRound;
    }

    void GameManager::restore(bool shouldInit)
    {
        if (shouldInit)
            initGame();
    }

    void GameManager::reset()
    {
        world.reset();
        gameState = GameState::None;
        round = 0;
        playerTurnOrder.clear();
        currentPlayerIndex = 0;
        exploreState = ExploreState::None;
        interactionFlags = InteractionFlag_None;
        inventory.reset();
        CombatSystem::getInstance()->reset();
    }

    const std::shared_ptr<GameManager> GameManager::getInstance()
    {
        static const auto instance = std::shared_ptr<GameManager>(new GameManager());
        return instance;
    }
    bool GameManager::isPosTraversable(const Vec2i &pos)
    {
        if (!world->inBound(pos))
            return false;
        return world->getRectAt(pos)->traversable();
    }
} // namespace FTK
