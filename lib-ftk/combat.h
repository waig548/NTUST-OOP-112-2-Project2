#ifndef FTK_COMBAT_H
#define FTK_COMBAT_H

#include <deque>
#include <queue>
#include <memory>

#include <nlohmann/json.hpp>

#include "Entity.h"
#include "Action.h"

namespace FTK
{
    struct ActionNode
    {
        std::shared_ptr<Action> action;
        std::string actionID;
        std::shared_ptr<Entity> source;
        std::shared_ptr<Entity> target;
        std::shared_ptr<Entity> mainTarget;
        std::deque<std::shared_ptr<ActionNode>> before;
        std::deque<std::shared_ptr<ActionNode>> after;
        std::shared_ptr<ActionNode> parent;

        bool fromActiveSkill() const;
        bool fromPassiveSkill() const;
        bool fromItem() const;
    };

    enum class CombatState
    {
        None,
        BeginRound,
        BeginTurn,
        ChooseAction,
        RollDice,
        ResolveActions,
        ProcessActions,
        EndTurn,
        EndRound,
        EndBattle
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(CombatState, {{CombatState::None, "none"},
                                               {CombatState::BeginRound, "begin_round"},
                                               {CombatState::BeginTurn, "begin_turn"},
                                               {CombatState::ChooseAction, "choose_action"},
                                               {CombatState::RollDice, "roll_dice"},
                                               {CombatState::ResolveActions, "resolve_actions"},
                                               {CombatState::ProcessActions, "process_actions"},
                                               {CombatState::EndTurn, "end_turn"},
                                               {CombatState::EndRound, "end_round"},
                                               {CombatState::EndBattle, "end_battle"}})

    enum class ActionSelectionType
    {
        Skill,
        Item
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(ActionSelectionType,
                                 {{ActionSelectionType::Skill, "skill"},
                                  {ActionSelectionType::Item, "item"}})

    class CombatSystem
    {
    public:
        CombatState getCombatState() const;
        std::vector<std::shared_ptr<Player>> getPlayers() const;
        std::vector<std::shared_ptr<Enemy>> getEnemies() const;
        std::vector<std::shared_ptr<Entity>> getEntities() const;
        size_t getRoundNumber() const;
        size_t getTurnNumber() const;
        std::map<uuids::uuid, size_t> getActionPerformed() const;
        std::vector<uuids::uuid> getPriorities() const;
        ActionSelectionType getActionSelectionType() const;
        std::vector<std::string> getActionCandidates() const;
        std::string getSelectedSctionID() const;
        std::vector<std::shared_ptr<Entity>> getTargetCandidates() const;
        uuids::uuid getSelectedTarget() const;

        bool shouldEndRound() const;
        bool shouldEndBattle() const;

        std::shared_ptr<Entity> getCurrentEntity() const;
        std::shared_ptr<Entity> getEntityByUUID(const uuids::uuid uuid) const;
        std::vector<std::shared_ptr<Entity>> getAlliesOf(const std::shared_ptr<Entity> &entity) const;
        std::vector<std::shared_ptr<Entity>> getEnemiesOf(const std::shared_ptr<Entity> &entity) const;
        std::vector<std::shared_ptr<Entity>> getCurrentAllies() const;
        std::vector<std::shared_ptr<Entity>> getCurrentEnemies() const;

        void beginBattle(std::vector<std::shared_ptr<Player>> players, std::vector<std::shared_ptr<Enemy>> enemies, bool ambushFailed = false);
        void beginRound();
        void beginTurn();
        void chooseAction();
        void selectSkill();
        void selectTarget();
        void rollDice();
        void resolveActions();
        void processActions();
        void endTurn();
        void endRound();
        void endBattle();

        void setActionSelectionType(ActionSelectionType type);

        void prepSelectAction();
        void markActionSelected(const std::string &actionID);

        void prepSelectTarget();
        void markTargetSelected(uuids::uuid uuid);

        bool readyToRollDice();

        void confirmChoice();

        void prepRollDice();
        void markDiceRolled(size_t rolledAmount);

        void reset();

        nlohmann::ordered_json saveState();
        void retoreState(const nlohmann::json &j);

        static std::shared_ptr<CombatSystem> getInstance();

    private:
        CombatSystem() = default;

        void updatePriorities();

        std::deque<std::shared_ptr<ActionNode>> resolveAction(const std::shared_ptr<ActionNode> &actionNode);
        void processAction(const std::shared_ptr<ActionNode> actionNode, ActionContext &ctx);
        CombatState combatState;
        size_t round;
        size_t turn;

        ActionSelectionType actionSelectionType;
        std::vector<std::string> actionCandidates;
        std::string selectedActionID;
        std::vector<uuids::uuid> targetCandidates;
        uuids::uuid selectedTarget;
        size_t diceRollResult;

        std::deque<std::deque<std::shared_ptr<ActionNode>>> actionGroupQueue;   // transient data, not saved in state

        std::set<uuids::uuid> playerDeaths;
        std::set<uuids::uuid> playersEscaped;
        std::set<uuids::uuid> enemyDeaths;

        std::map<uuids::uuid, size_t> actionPerformed;
        std::vector<uuids::uuid> priorities;
        std::vector<std::shared_ptr<Player>> players;
        std::vector<std::shared_ptr<Enemy>> enemies;
    };
} // namespace FTK

#endif // FTK_COMBAT_H
