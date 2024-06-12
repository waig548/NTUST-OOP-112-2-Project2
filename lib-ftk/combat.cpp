#include "combat.h"

#include <stack>

#include <effolkronium/random.hpp>

#include "utils.h"
#include "Dice.h"
#include "Registry.h"
#include "GameManager.h"
#include "Serializer.h"

template <typename R, typename T>
static std::vector<std::shared_ptr<R>> vectorCastSharedPtrTo(const std::vector<std::shared_ptr<T>> vec)
{
    return FTK::map<std::shared_ptr<R>>(vec, [](std::shared_ptr<T> ptr)
                                        { return std::dynamic_pointer_cast<R>(ptr); });
}
namespace FTK
{
    CombatState CombatSystem::getCombatState() const
    {
        return combatState;
    }

    std::vector<std::shared_ptr<Player>> CombatSystem::getPlayers() const
    {
        return players;
    }

    std::vector<std::shared_ptr<Enemy>> CombatSystem::getEnemies() const
    {
        return enemies;
    }

    std::vector<std::shared_ptr<Entity>> CombatSystem::getEntities() const
    {
        std::vector<std::shared_ptr<Entity>> res;
        for (auto ep : players)
            res.push_back(std::dynamic_pointer_cast<Entity>(ep));
        for (auto en : enemies)
            res.push_back(std::dynamic_pointer_cast<Entity>(en));
        return res;
    }

    size_t CombatSystem::getRoundNumber() const
    {
        return round;
    }

    size_t CombatSystem::getTurnNumber() const
    {
        return turn;
    }

    std::map<uuids::uuid, size_t> CombatSystem::getActionPerformed() const
    {
        return actionPerformed;
    }

    std::vector<uuids::uuid> CombatSystem::getPriorities() const
    {
        return priorities;
    }

    ActionSelectionType CombatSystem::getActionSelectionType() const
    {
        return actionSelectionType;
    }

    std::vector<std::string> CombatSystem::getActionCandidates() const
    {
        return actionCandidates;
    }

    std::string CombatSystem::getSelectedSctionID() const
    {
        return selectedActionID;
    }

    std::vector<std::shared_ptr<Entity>> CombatSystem::getTargetCandidates() const
    {
        return filter(getEntities(), [this](auto e)
                      { return std::find(targetCandidates.begin(), targetCandidates.end(), e->uuid) != targetCandidates.end(); });
    }

    uuids::uuid CombatSystem::getSelectedTarget() const
    {
        return selectedTarget;
    }

    bool CombatSystem::shouldEndRound() const
    {
        return std::count_if(actionPerformed.begin(), actionPerformed.end(), [](auto p)
                             { return p.second; }) >= actionPerformed.size();
    }

    bool CombatSystem::shouldEndBattle() const
    {
        return players.empty() || enemies.empty();
    }

    std::shared_ptr<Entity> CombatSystem::getCurrentEntity() const
    {
        if (priorities.empty())
            return nullptr;
        for (auto e : getEntities())
            if (e->uuid == priorities.front())
                return e;
        return nullptr;
    }

    std::shared_ptr<Entity> CombatSystem::getEntityByUUID(const uuids::uuid uuid) const
    {
        auto entities = getEntities();
        if (auto it = std::find_if(entities.begin(), entities.end(), [uuid](auto e)
                                   { return e->uuid == uuid; });
            it != entities.end())
            return *it;
        return nullptr;
    }

    std::vector<std::shared_ptr<Entity>> CombatSystem::getAlliesOf(const std::shared_ptr<Entity> &entity) const
    {
        if (entity->isEnemy())
            return vectorCastSharedPtrTo<Entity>(enemies);
        if (entity->isPlayer())
            return vectorCastSharedPtrTo<Entity>(players);
        return {};
    }

    std::vector<std::shared_ptr<Entity>> CombatSystem::getEnemiesOf(const std::shared_ptr<Entity> &entity) const
    {
        if (entity->isEnemy())
            return vectorCastSharedPtrTo<Entity>(players);
        else if (entity->isPlayer())
            return vectorCastSharedPtrTo<Entity>(enemies);
        return {};
    }

    std::vector<std::shared_ptr<Entity>> CombatSystem::getCurrentAllies() const
    {
        return getAlliesOf(getCurrentEntity());
    }

    std::vector<std::shared_ptr<Entity>> CombatSystem::getCurrentEnemies() const
    {
        return getEnemiesOf(getCurrentEntity());
    }

    void CombatSystem::beginBattle(std::vector<std::shared_ptr<Player>> players, std::vector<std::shared_ptr<Enemy>> enemies, bool ambushFailed)
    {
        if (combatState == CombatState::None)
        {
            reset();
            this->players = players;
            this->enemies = enemies;
            if (ambushFailed)
            {
                auto speedUp = MainRegistry::getInstance()->buffTemplates->get("buff:speed_up");
                for (auto enm : this->enemies)
                    enm->addBuff(speedUp.build(2));
            }
            combatState = CombatState::BeginRound;
        }
    }

    void CombatSystem::beginRound()
    {
        if (combatState == CombatState::BeginRound)
        {
            actionPerformed.clear();
            for (auto ent : getEntities())
                actionPerformed.try_emplace(ent->uuid, 0);
            updatePriorities();
            round++;
            combatState = CombatState::BeginTurn;
        }
    }

    void CombatSystem::beginTurn()
    {
        if (combatState == CombatState::BeginTurn)
        {
            turn++;
            getCurrentEntity()->updateSkillCD();
            auto buffs = MainRegistry::getInstance()->buffTemplates;
            for (auto b : getCurrentEntity()->getBuffs())
            {
                if (buffs->get(b.id).effectType == EffectType::SkipTurn)
                {
                    combatState = CombatState::EndTurn;
                    return;
                }
            }

            setActionSelectionType(ActionSelectionType::Skill);
            combatState = CombatState::ChooseAction;
            if (getCurrentEntity()->isEnemy())
                chooseAction();
        }
    }

    void CombatSystem::chooseAction()
    {
        if (combatState == CombatState::ChooseAction)
        {
            selectSkill();
            selectTarget();
        }
    }

    void CombatSystem::selectSkill()
    {
        if (combatState == CombatState::ChooseAction && !actionCandidates.empty())
        {
            std::vector<std::string> box = actionCandidates;
            effolkronium::random_static::shuffle(box);
            selectedActionID = box.front();
            prepSelectTarget();
        }
    }

    void CombatSystem::selectTarget()
    {
        if (combatState == CombatState::ChooseAction && !targetCandidates.empty())
        {
            std::vector<uuids::uuid> box = targetCandidates;
            effolkronium::random_static::shuffle(box);
            selectedTarget = box.front();
        }
    }

    void CombatSystem::rollDice()
    {
        if (combatState == CombatState::RollDice)
        {
            auto skillData = MainRegistry::getInstance()->activeSkills->get(selectedActionID);
            auto ent = getCurrentEntity();
            auto rollChance = skillData.rollChanceExpr.eval(ent->getMathContext("self")) / 100;
            auto diceRolls = skillData.diceRolls;
            if (skillData.id == "active:basic_attack")
                diceRolls = ent->getWeaponDiceRoll();
            markDiceRolled(Dice::rollUniformDices(diceRolls, rollChance));
        }
    }

    void CombatSystem::resolveActions()
    {
        if (combatState == CombatState::ResolveActions)
        {
            std::deque<std::deque<std::shared_ptr<ActionNode>>> actionGroups;
            if (!selectedActionID.empty() && actionSelectionType == ActionSelectionType::Skill)
            {
                auto skillData = MainRegistry::getInstance()->activeSkills->get(selectedActionID);
                std::deque<std::shared_ptr<ActionNode>> actions;
                for (auto act : skillData.actions)
                {
                    auto node = std::make_shared<ActionNode>();
                    node->action = act;
                    node->actionID = selectedActionID;
                    node->source = getCurrentEntity();
                    node->target = getEntityByUUID(selectedTarget);
                    node->mainTarget = getEntityByUUID(selectedTarget);
                    actions.push_back(node);
                }
                actionGroups.push_back(actions);
            }
            else if (!selectedActionID.empty() && actionSelectionType == ActionSelectionType::Item)
            {
                auto itemData = MainRegistry::getInstance()->itemTemplates->get(selectedActionID);
                std::deque<std::shared_ptr<ActionNode>> actions;
                for (auto act : itemData.actionsOnUse)
                {
                    auto node = std::make_shared<ActionNode>();
                    node->action = act;
                    node->actionID = selectedActionID;
                    node->source = getCurrentEntity();
                    node->target = getEntityByUUID(selectedTarget);
                    node->mainTarget = getEntityByUUID(selectedTarget);
                    actions.push_back(node);
                }
                actionGroups.push_back(actions);
            }
            if (auto buffs = MainRegistry::getInstance()->buffTemplates)
            {
                for (auto b : getCurrentEntity()->getBuffs())
                {
                    std::deque<std::shared_ptr<ActionNode>> actions;
                    auto buff = buffs->get(b.id);
                    for (auto act : buff.actions)
                    {
                        auto node = std::make_shared<ActionNode>();
                        node->action = act;
                        node->source = getCurrentEntity();
                        node->target = getEntityByUUID(selectedTarget);
                        actions.push_back(node);
                    };
                    actionGroups.push_back(actions);
                }
            }

            for (auto curActions : actionGroups)
            {
                std::deque<std::shared_ptr<ActionNode>> nodes;

                for (auto act : curActions)
                {
                    nodes = resolveAction(act);
                }
                actionGroupQueue.push_back(nodes);
            }
            combatState = CombatState::ProcessActions;
            processActions();
        }
    }

    void CombatSystem::processActions()
    {
        if (combatState == CombatState::ProcessActions)
        {
            for (auto actionGroup : actionGroupQueue)
            {
                ActionContext ctx;
                ctx.mathContext = Math::Context::default_global();
                for (auto action : actionGroup)
                {
                    processAction(action, ctx);
                }
                if (!actionGroup.empty() && actionGroup.front()->fromItem())
                    GameManager::getInstance()->getInventory()->removeItem(actionGroup.front()->actionID);
            }
            actionGroupQueue.clear();
            combatState = CombatState::EndTurn;
        }
    }

    void CombatSystem::endTurn()
    {
        if (combatState == CombatState::EndTurn)
        {
            actionPerformed[getCurrentEntity()->uuid]++;
            for (auto e : players)
                if (e->isDead())
                    playerDeaths.insert(e->uuid);

            for (auto e : enemies)
                if (e->isDead())
                    enemyDeaths.insert(e->uuid);

            for (auto uuid : playerDeaths)
            {
                if (auto it = std::find_if(players.begin(), players.end(), [uuid](auto e)
                                           { return e->uuid == uuid; });
                    it != players.end())
                    players.erase(it);
                actionPerformed.erase(uuid);
            }
            for (auto uuid : playersEscaped)
            {
                if (auto it = std::find_if(players.begin(), players.end(), [uuid](auto e)
                                           { return e->uuid == uuid; });
                    it != players.end())
                {
                    (*it)->clearBuffs();
                    players.erase(it);
                }
                actionPerformed.erase(uuid);
            }
            for (auto uuid : enemyDeaths)
            {
                if (auto it = std::find_if(enemies.begin(), enemies.end(), [uuid](auto e)
                                           { return e->uuid == uuid; });
                    it != enemies.end())
                    enemies.erase(it);
                actionPerformed.erase(uuid);
            }

            if (getCurrentEntity())
                getCurrentEntity()->updateBuffs();

            for (auto ent : getEntities())
            {
                ent->checkBuffs();
            }

            priorities.clear();
            actionCandidates.clear();
            selectedActionID = {};
            targetCandidates.clear();
            selectedTarget = {};
            diceRollResult = {};

            actionGroupQueue = {};

            if (shouldEndBattle() || shouldEndRound())
                combatState = CombatState::EndRound;
            else
            {
                updatePriorities();
                combatState = CombatState::BeginTurn;
            }
        }
    }

    void CombatSystem::endRound()
    {
        if (combatState == CombatState::EndRound)
        {
            if (shouldEndBattle())
                combatState = CombatState::EndBattle;
            else
            {
                combatState = CombatState::BeginRound;
            }
        }
    }

    void CombatSystem::endBattle()
    {
        if (combatState == CombatState::EndBattle)
        {
            auto gameMgr = GameManager::getInstance();
            for (auto uuid : playerDeaths)
                gameMgr->markPlayerDead(uuid);
            for (auto uuid : enemyDeaths)
                gameMgr->markEntityDead(uuid);

            for (auto ent : getEntities())
            {
                ent->clearBuffs();
            }

            reset();

            gameMgr->markInteractionDone();
        }
    }

    void CombatSystem::setActionSelectionType(ActionSelectionType type)
    {
        actionSelectionType = type;
        prepSelectAction();
    }

    void CombatSystem::prepSelectAction()
    {
        actionCandidates.clear();
        selectedActionID = {};
        if (actionSelectionType == ActionSelectionType::Skill)
        {
            auto ent = getCurrentEntity();
            for (auto p : ent->getSkillCD())
            {
                if (p.first._Starts_with("active") && !p.second)
                    actionCandidates.push_back(p.first);
            }
        }
        else if (actionSelectionType == ActionSelectionType::Item)
        {
            auto inv = GameManager::getInstance()->getInventory();
            auto itemData = MainRegistry::getInstance()->itemTemplates;
            for (auto item : inv->getItems())
            {
                if (itemData->get(item.id).availableInCombat)
                    actionCandidates.push_back(item.id);
            }
        }
    }

    void CombatSystem::markActionSelected(const std::string &actionID)
    {
        selectedActionID = actionID;
        prepSelectTarget();
    }

    void CombatSystem::prepSelectTarget()
    {
        if (selectedActionID.empty())
            return;
        if (actionSelectionType == ActionSelectionType::Skill)
        {
            auto skillData = MainRegistry::getInstance()->activeSkills->get(selectedActionID);
            auto curEnt = getCurrentEntity();
            switch (skillData.targetType)
            {
            case TargetType::None:
                targetCandidates = {};
                break;
            case TargetType::Self:
                targetCandidates = {priorities.front()};
                break;
            case TargetType::Single:
            case TargetType::Multi:
            case TargetType::SplashExcludingMain:
            case TargetType::Splash:
                if (skillData.targetScope == TargetScope::Ally)
                {
                    targetCandidates = map<uuids::uuid>(getCurrentAllies(), [](auto e)
                                                        { return e->uuid; });
                }
                else if (skillData.targetScope == TargetScope::Enemy)
                {
                    targetCandidates = map<uuids::uuid>(getCurrentEnemies(), [](auto e)
                                                        { return e->uuid; });
                }
                else if (skillData.targetScope == TargetScope::All)
                {
                    targetCandidates = map<uuids::uuid>(getEntities(), [](auto e)
                                                        { return e->uuid; });
                }
                break;
            default:
                break;
            }
        }
        else if (actionSelectionType == ActionSelectionType::Item)
        {
            targetCandidates = {priorities.front()};
        }
        selectedTarget = {};
    }

    void CombatSystem::markTargetSelected(uuids::uuid uuid)
    {
        selectedTarget = uuid;
    }

    bool CombatSystem::readyToRollDice()
    {
        if (selectedActionID.empty())
            return false;
        if (actionSelectionType == ActionSelectionType::Skill && MainRegistry::getInstance()->activeSkills->get(selectedActionID).targetType == TargetType::None)
            return true;
        return !selectedTarget.is_nil();
    }

    void CombatSystem::confirmChoice()
    {
        prepRollDice();
        if (getCurrentEntity()->isEnemy())
            rollDice();
    }

    void CombatSystem::prepRollDice()
    {
        combatState = CombatState::RollDice;
    }

    void CombatSystem::markDiceRolled(size_t rolledAmount)
    {
        diceRollResult = rolledAmount;
        combatState = CombatState::ResolveActions;
    }

    void CombatSystem::reset()
    {
        combatState = CombatState::None;
        round = 0;
        turn = 0;

        actionCandidates.clear();
        selectedActionID = {};
        targetCandidates.clear();
        selectedTarget = {};
        diceRollResult = 0;

        actionGroupQueue = {};

        playerDeaths.clear();
        enemyDeaths.clear();
        playersEscaped.clear();

        actionPerformed.clear();
        priorities.clear();
        players.clear();
        enemies.clear();
    }

    nlohmann::ordered_json CombatSystem::saveState()
    {
        auto res = nlohmann::ordered_json::object();
        if (combatState != CombatState::None)
        {
            res["combat_state"] = combatState;
            res["round"] = round;
            res["turn"] = turn;
            res["action_selection_type"] = actionSelectionType;
            res["skill_candidates"] = actionCandidates;
            res["selected_skill_id"] = selectedActionID;
            res["target_candidates"] = targetCandidates;
            res["selected_target"] = selectedTarget;
            res["dice_roll_result"] = diceRollResult;
            res["player_deaths"] = playerDeaths;
            res["enemy_deaths"] = enemyDeaths;
            res["players_escaped"] = playersEscaped;
            res["action_performed"] = actionPerformed;
            res["priorities"] = priorities;
            res["players"] = map<uuids::uuid>(players, [](auto ep)
                                              { return ep->uuid; });
            res["enemies"] = map<uuids::uuid>(enemies, [](auto en)
                                              { return en->uuid; });
        }
        return res;
    }

    void CombatSystem::retoreState(const nlohmann::json &j)
    {
        if (!j.empty())
        {
            reset();
            combatState = j["combat_state"];
            round = j["round"];
            turn = j["turn"];
            actionSelectionType = j["action_selection_type"];
            actionCandidates = j["skill_candidates"];
            selectedActionID = j["selected_skill_id"];
            targetCandidates = j["target_candidates"];
            selectedTarget = j["selected_target"];
            diceRollResult = j["dice_roll_result"];
            playerDeaths = j["player_deaths"];
            enemyDeaths = j["enemy_deaths"];
            playersEscaped = j["players_escaped"];
            actionPerformed = j["action_performed"];
            priorities = j["priorities"];
            auto world = GameManager::getInstance()->getWorld();
            auto puuids = j["players"].get<std::vector<uuids::uuid>>();
            auto euuids = j["enemies"].get<std::vector<uuids::uuid>>();
            for (auto u : puuids)
                players.push_back(world->getPlayerByUUID(u));
            std::vector<std::shared_ptr<Entity>> tmp;
            for (auto u : euuids)
                tmp.push_back(world->getEntityByUUID(u));
            enemies = vectorCastSharedPtrTo<Enemy>(tmp);
        }
    }

    std::shared_ptr<CombatSystem> CombatSystem::getInstance()
    {
        static auto instance = std::shared_ptr<CombatSystem>(new CombatSystem());
        return instance;
    }

    void CombatSystem::updatePriorities()
    {
        static auto calcPri = [this](auto ent) -> int
        {
            return (int)((actionPerformed[ent->uuid] + 1) / ent->get("speed") * 100);
        };

        auto ents = getEntities();
        std::stable_sort(ents.begin(), ents.end(), [](auto e1, auto e2)
                         { return e1->uuid < e2->uuid; });
        std::stable_sort(ents.begin(), ents.end(), [](auto e1, auto e2)
                         { return e1->get("max_hp") > e2->get("max_hp"); });
        std::stable_sort(ents.begin(), ents.end(), [](auto e1, auto e2)
                         { return (e1->get("p_def") + e1->get("m_def")) > (e2->get("p_def") + e2->get("m_def")); });
        std::stable_sort(ents.begin(), ents.end(), [](auto e1, auto e2)
                         { return (e1->get("p_atk") + e1->get("m_atk")) > (e2->get("p_atk") + e2->get("m_atk")); });
        std::stable_sort(ents.begin(), ents.end(), [](auto e1, auto e2)
                         { return e1->get("speed") > e2->get("speed"); });
        std::stable_sort(ents.begin(), ents.end(), [](auto e1, auto e2)
                         { return calcPri(e1) < calcPri(e2); });

        priorities = map<uuids::uuid>(ents, [](auto ent)
                                      { return ent->uuid; });
    }

    std::deque<std::shared_ptr<ActionNode>> CombatSystem::resolveAction(const std::shared_ptr<ActionNode> &actionNode)
    {
        std::deque<std::shared_ptr<ActionNode>> expandedAction;

        if (actionNode->parent && actionNode->actionID == actionNode->parent->actionID)
            return expandedAction;

        auto curAction = actionNode->action;
        if (curAction->targetType == TargetType::None)
        {
        }
        else if (curAction->targetType == TargetType::Self)
        {
            auto node = std::make_shared<ActionNode>(*actionNode);
            node->target = actionNode->source;
            expandedAction.push_back(node);
        }
        else if (curAction->targetType == TargetType::Single)
        {
            auto node = std::make_shared<ActionNode>(*actionNode);
            expandedAction.push_back(node);
        }
        else if (curAction->targetType == TargetType::Main && actionNode->mainTarget)
        {
            auto node = std::make_shared<ActionNode>(*actionNode);
            node->target = actionNode->mainTarget;
            expandedAction.push_back(node);
        }
        else if (curAction->targetType == TargetType::SplashExcludingMain && actionNode->mainTarget)
        {
            auto targets = getAlliesOf(actionNode->mainTarget);
            targets.erase(std::remove(targets.begin(), targets.end(), actionNode->mainTarget));

            for (auto e : targets)
            {
                auto node = std::make_shared<ActionNode>(*actionNode);
                node->target = e;
                expandedAction.push_back(node);
            }
        }

        else if (curAction->targetType == TargetType::Splash)
        {
            std::vector<std::shared_ptr<Entity>> targets;
            if (curAction->targetScope == TargetScope::Enemy)
                targets = getEnemiesOf(actionNode->source);
            else if (curAction->targetScope == TargetScope::Ally)
                targets = getAlliesOf(actionNode->source);
            else if (curAction->targetScope == TargetScope::All)
                targets = getEntities();

            for (auto e : targets)
            {
                auto node = std::make_shared<ActionNode>(*actionNode);
                node->target = e;
                expandedAction.push_back(node);
            }
        }

        auto passivesData = MainRegistry::getInstance()->passiveSkills;
        auto diceRolled = diceRollResult;

        for (auto &cur : expandedAction)
        {
            auto collectPassives = [diceRolled, &cur, &passivesData](std::shared_ptr<Entity> ent, bool isTarget = false)
            {
                auto passiveCDs = ent->getPassiveSkillCD();
                for (auto p : passiveCDs)
                {
                    if (p.second)
                        continue;
                    auto passive = passivesData->get(p.first);
                    if ((passive.id != cur->actionID && (!passive.requireActiveSkill || cur->fromActiveSkill())))
                    {

                        for (auto act : passive.actions)
                        {
                            auto node = std::make_shared<ActionNode>(*cur);
                            node->action = act;
                            if (isTarget)
                            {
                                node->source = ent;
                                node->target = {};
                            }
                            node->actionID = passive.id;
                            node->parent = cur;
                            node->before.clear();
                            node->after.clear();

                            if (cur->action->actionType == ActionType::Damage)
                            {
                                if (passive.triggerType == PassiveTriggerType::BeforeAttack && !isTarget)
                                    cur->before.push_back(node);
                                else if (passive.triggerType == PassiveTriggerType::BeforeAttacked && isTarget)
                                    cur->before.push_back(node);
                                else if (passive.triggerType == PassiveTriggerType::OnDamaged && isTarget && diceRolled)
                                    cur->before.push_back(node);
                                else if (passive.triggerType == PassiveTriggerType::AfterDamaged && isTarget && diceRolled)
                                    cur->before.push_back(node);
                                else if (passive.triggerType == PassiveTriggerType::AfterAttack && !isTarget)
                                    cur->after.push_back(node);
                                else if (passive.triggerType == PassiveTriggerType::AfterAttacked && isTarget)
                                    cur->after.push_back(node);
                            }
                        }
                    }
                }
            };

            collectPassives(cur->source);
            if (cur->source->uuid != cur->target->uuid)
                collectPassives(cur->target, true);

            std::deque<std::shared_ptr<ActionNode>> res;
            for (auto &sub : cur->before)
            {
                for (auto result : resolveAction(sub))
                    res.push_back(result);
            }
            cur->before = res;
            res.clear();

            for (auto &sub : cur->after)
            {
                for (auto result : resolveAction(sub))
                    res.push_back(result);
            }
            cur->after = res;
            res.clear();
        }
        return expandedAction;
    }

    void CombatSystem::processAction(const std::shared_ptr<ActionNode> actionNode, ActionContext &ctx)
    {
        if (!actionNode->parent)
        {
            ctx.mathContext = actionNode->source->getMathContext("self", ctx.mathContext);
            ctx.mathContext = actionNode->target->getMathContext("target", ctx.mathContext);
            if (actionNode->fromActiveSkill())
                ctx.mathContext = MainRegistry::getInstance()->activeSkills->get(actionNode->actionID).getContext("skill", ctx.mathContext);
            ctx.mathContext = actionNode->action->getContext("sourceAction", ctx.mathContext);
            if (actionNode->actionID == "active:basic_attack")
                ctx.mathContext["dice_rolls"] = actionNode->source->getWeaponDiceRoll();
            ctx.mathContext["rolled_result"] = diceRollResult;
        }

        for (auto beforeAct : actionNode->before)
        {
            ctx.mathContext = actionNode->action->getContext("sourceAction", ctx.mathContext);
            processAction(beforeAct, ctx);
        }

        ctx.mathContext = actionNode->source->getMathContext("self", ctx.mathContext);
        ctx.mathContext = actionNode->target->getMathContext("target", ctx.mathContext);
        if (actionNode->fromActiveSkill())
        {
            ctx.mathContext = MainRegistry::getInstance()->activeSkills->get(actionNode->actionID).getContext("skill", ctx.mathContext);
            if (actionNode->actionID == "active:basic_attack")
                ctx.mathContext["dice_rolls"] = actionNode->source->getWeaponDiceRoll();
        }
        if (actionNode->fromPassiveSkill())
            ctx.condition = MainRegistry::getInstance()->passiveSkills->get(actionNode->actionID).condition;
        else
            ctx.condition = Math::Condition("1");
        auto doEffect = actionNode->action->shouldHaveEffect(ctx, actionNode->fromActiveSkill());
        if (doEffect)
        {
            if (actionNode->action->actionType == ActionType::Flee)
                playersEscaped.insert(actionNode->target->uuid);
            actionNode->action->apply(actionNode->source, actionNode->target, ctx, actionNode->fromActiveSkill());

            if (actionNode->fromPassiveSkill())
                actionNode->source->resetSkillCD(actionNode->actionID);
        }
        if (actionNode->fromActiveSkill())
            actionNode->source->resetSkillCD(actionNode->actionID);
        if (actionNode->before.size())
        {
            actionNode->source->checkBuffs(-1);
            actionNode->target->checkBuffs(-1);
        }
        for (auto afterAct : actionNode->after)
            processAction(afterAct, ctx);
        if (actionNode->after.size())
        {
            actionNode->source->checkBuffs(-1);
            actionNode->target->checkBuffs(-1);
        }
    }

    bool ActionNode::fromActiveSkill() const
    {
        return actionID._Starts_with("active");
    }

    bool ActionNode::fromPassiveSkill() const
    {
        return actionID._Starts_with("passive");
    }

    bool ActionNode::fromItem() const
    {
        return actionID._Starts_with("item");
    }

} // namespace FTK
