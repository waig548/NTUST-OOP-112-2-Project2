#if !defined(FTK_GAME_MANAGER_H)
#define FTK_GAME_MANAGER_H

#include "World.h"
#include "Inventory.h"

namespace FTK
{
    enum class GameState
    {
        None,
        Explore,
        Interact,
        Shop,
        Teleport,
        Battle
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(GameState,
                                 {{GameState::None, "none"},
                                  {GameState::Explore, "explore"},
                                  {GameState::Interact, "interact"},
                                  {GameState::Shop, "shop"},
                                  {GameState::Teleport, "teleport"},
                                  {GameState::Battle, "battle"}})

    enum class ExploreState
    {
        None,
        BeginRound,
        BeginTurn,
        RollAP,
        Move,
        EndTurn,
        EndRound
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(ExploreState,
                                 {{ExploreState::None, "none"},
                                  {ExploreState::BeginRound, "begin_round"},
                                  {ExploreState::BeginTurn, "begin_turn"},
                                  {ExploreState::RollAP, "roll_ap"},
                                  {ExploreState::Move, "move"},
                                  {ExploreState::EndTurn, "end_turn"},
                                  {ExploreState::EndRound, "end_round"}})

    enum class InteractableType
    {
        None,
        Enemy,
        RE
    };

    enum InteractionFlag
    {
        InteractionFlag_None = 0,
        InteractionFlag_InteractedWithEnemy = 1 << 0,
        InteractionFlag_InteractedWithRE = 1 << 1
    };

    using InteractionFlags = size_t;

    class GameManager
    {
    public:
        std::shared_ptr<World> getWorld() const;

        GameState getGameState() const;
        size_t getRoundNumber() const;
        std::vector<uuids::uuid> getPlayerTurnOrder() const;
        size_t getCurrentPlayerIndex() const;
        uuids::uuid getCurrentPlayerUUID() const;
        std::shared_ptr<Player> getCurrentPlayer() const;
        ExploreState getExploreState() const;
        std::shared_ptr<Inventory> getInventory() const;

        bool shouldEndMoveState() const;
        InteractableType getInteractableType(const Vec2i &pos) const;
        InteractableType getCurrentInteractableType() const;

        void beginTurn();
        void rollAP(const uuids::uuid uuid, int focusUsed);
        bool movePlayer(const uuids::uuid &uuid, const Vec2i &newPos, bool retreat = false, bool ignoreAdjacent = false);
        bool retreatPlayer(const uuids::uuid &uuid);
        void endTurn();

        void rollAP(int focusUsed);
        bool movePlayer(const Vec2i &newPos, bool retreat = false, bool ignoreAdjacent = false);
        bool retreatPlayer();

        void beginRound();
        void endRound();

        void enterShop();
        void exitShop();

        void beginTeleport();
        void endTeleport();

        void useItem(const std::string &itemID);

        void triggerBattle(const uuids::uuid &triggerer, const Vec2i &pos, bool ambush = false, bool ambushFailed = false);
        void triggerBattle(const Vec2i &pos, bool ambush = false, bool ambushFailed = false);

        void prepAPRoll();
        void markAPRolled(size_t rolledAmount);

        void markInteractionDone(bool all = false);

        void markEntityDead(const uuids::uuid &entityUUID);
        void markPlayerDead(const uuids::uuid &playerUUID);

        void saveMap(const std::string &path);
        void saveMap(const char *path);

        void loadMap(const std::string &path);
        void loadMap(const char *path);

        void initGame();
        void restore(bool shouldInit = false);
        void reset();

        static const std::shared_ptr<GameManager> getInstance();

    private:
        GameManager() = default;

        bool isPosTraversable(const Vec2i &pos);

        std::shared_ptr<World> world;
        GameState gameState;
        size_t round;
        std::vector<uuids::uuid> playerTurnOrder;

        size_t currentPlayerIndex;
        ExploreState exploreState;
        InteractionFlags interactionFlags;
        std::shared_ptr<Inventory> inventory;
    };

} // namespace FTK

#endif // FTK_GAME_MANAGER_H
