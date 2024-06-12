#include "view.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include "utils.h"
#include "Dice.h"
#include "GameManager.h"
#include "Registry.h"
#include "combat.h"

#include "gui.h"

static const std::vector<std::pair<std::string, std::string>> EntityStatViewKeys{
    {"p_atk", "Physical ATK"},
    {"m_atk", "Magical ATK"},
    {"p_def", "Physical DEF"},
    {"m_def", "Magical DEF"},
    {"hit_rate", "Hit Rate"},
    {"speed", "Speed"}};

static const std::map<FTK::EquipmentType, std::string> equipmentTexts{
    {FTK::EquipmentType::None, "None"},
    {FTK::EquipmentType::Weapon, "Weapon"},
    {FTK::EquipmentType::Armor, "Armor"},
    {FTK::EquipmentType::Accessory, "Bauble"}};

static bool shouldFocus = false;
static ImVec2 gameViewFocus;
static bool doneRolling = false;
size_t diceRollRes = 0;
static int focusUsed = 0;
static bool invalidPos = false;
static bool doneInteration = false;

static ImGuiFileDialog fileDialog;
static IGFD::FileDialogConfig config;
namespace FTK::GUI
{
    void ViewManager::render()
    {
        switch (viewState)
        {
        case ViewState::MainMenu:
            renderMainMenu();
            break;
        case ViewState::NewGame:
            renderNewGame();
            break;
        case ViewState::InGame:
            renderInGame();
            break;
        default:
            break;
        }
    }

    std::shared_ptr<ViewManager> ViewManager::getInstance()
    {
        static std::shared_ptr<ViewManager> instance(new ViewManager());
        return instance;
    }

    ViewManager::ViewManager()
    {
        config.path = "./saves";
    }

    void ViewManager::renderMap(bool disabled)
    {
        if (shouldFocus)
        {
            ImGui::SetNextWindowScroll(gameViewFocus);
            shouldFocus = false;
        }
        ImGUI_FullscreenNextWindow();
        ImGui::Begin("##game_view", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBackground);

        ImGui::BeginDisabled(disabled);

        auto gameMgr = GameManager::getInstance();
        auto world = gameMgr->getWorld();
        for (int y = 0; y < world->dimension.getY(); y++)
        {
            for (int x = 0; x < world->dimension.getX(); x++)
            {
                if (x)
                    ImGui::SameLine();
                auto rect = world->getRectAt(x, y);
                Texture tex = getRectTexture(rect->getID(), rect->getMetadata());
                ImVec4 bgColor = {0, 0, 0, 0};
                std::string id = "rect_" + std::to_string(x) + "_" + std::to_string(y);
                auto cursor = ImGui::GetCursorPos();
                ImGui::SetNextItemAllowOverlap();
                if (!rect->isVisible())
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0);
                ImGui::Image((void *)(intptr_t)tex.textureID, ImVec2(tex.width, tex.height), ImVec2(0, 0), ImVec2(1, 1));
                if (!rect->isVisible())
                    ImGui::PopStyleVar();
                ImGui::SetCursorPos(cursor);
                std::string content;
                if (rect->isVisible())
                {
                    if (auto re = world->getRectEntityAt(x, y))
                    {
                        content += re->name + '\n';
                        if (re->type == RectEntityType::Shop)
                        {
                            Texture icon = getRectIconTexture("shop");
                            auto cursor = ImGui::GetCursorPos();
                            ImGui::SetNextItemAllowOverlap();
                            ImGui::Image((void *)(intptr_t)icon.textureID, ImVec2(icon.width, icon.height), ImVec2(0, 0), ImVec2(1, 1));
                            ImGui::SetCursorPos(cursor);
                        }
                    }
                    if (auto e = world->getEntitiesAt(x, y); e.size())
                    {
                        for (auto n : e)
                            content += n->name + '\n';
                        Texture icon = getRectIconTexture("enemy");
                        auto cursor = ImGui::GetCursorPos();
                        ImGui::SetNextItemAllowOverlap();
                        ImGui::Image((void *)(intptr_t)icon.textureID, ImVec2(icon.width, icon.height), ImVec2(0, 0), ImVec2(1, 1));
                        ImGui::SetCursorPos(cursor);
                    }
                    if (auto ep = world->getPlayersAt(x, y); ep.size())
                    {
                        for (auto e : ep)
                            content += e->name + '\n';
                        Texture icon = getRectIconTexture("player");
                        auto cursor = ImGui::GetCursorPos();
                        ImGui::SetNextItemAllowOverlap();
                        ImGui::Image((void *)(intptr_t)icon.textureID, ImVec2(icon.width, icon.height), ImVec2(0, 0), ImVec2(1, 1));
                        ImGui::SetCursorPos(cursor);
                    }
                }
                if (!content.empty())
                    content.pop_back();
                ImGui::BeginDisabled(!rect->isVisible() && gameMgr->getGameState() != GameState::Teleport);
                ImGui::PushID(id.c_str());
                ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
                if (ImGui::Button("", {64, 64}))
                {
                    std::cout << "clicked on " << id << std::endl;
                    if ((gameMgr->getGameState() == GameState::Teleport || (gameMgr->getGameState() == GameState::Explore && gameMgr->getExploreState() == ExploreState::Move)) && !gameMgr->movePlayer({x, y}))
                    {
                        invalidPos = true;
                    }
                }
                if (rect->isVisible())
                    ImGui::SetItemTooltip("(%d, %d)\n%s", x, y, content.c_str());
                else
                    ImGui::SetItemTooltip("(%d, %d)", x, y);
                ImGui::PopStyleColor();
                ImGui::PopID();
                ImGui::EndDisabled();
            }
        }

        ImGui::EndDisabled();

        ImGui::End();
    }

    void ViewManager::renderEntityInfo(std::shared_ptr<Entity> ent, bool inCombat)
    {
        auto combatSys = CombatSystem::getInstance();
        auto gameMgr = GameManager::getInstance();

        ImGui::Begin(ent->name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::BeginDisabled(ent->isDead());

        ImGui::Text("HP:");
        ImGui::SameLine();
        ImGui::ProgressBar(ent->get("hp") / std::max(ent->get("max_hp"), 1.0), {100, 0});
        ImGui::SameLine();
        ImGui::Text("%d / %d", ent->getAsInt("hp"), ent->getAsInt("max_hp"));
        if (ent->isPlayer())
        {
            ImGui::Text("AP:");
            ImGui::SameLine();
            ImGui::ProgressBar(ent->get("ap") / std::max(ent->get("max_ap"), 1.0), {100, 0});
            ImGui::SameLine();
            ImGui::Text("%d / %d", ent->getAsInt("ap"), ent->getAsInt("max_ap"));
            ImGui::Text("Focus:");
            ImGui::SameLine();
            ImGui::ProgressBar(ent->get("focus") / std::max(ent->get("max_focus"), 1.0), {100, 0});
            ImGui::SameLine();
            ImGui::Text("%d / %d", ent->getAsInt("focus"), ent->getAsInt("max_focus"));
            if (!inCombat)
            {
                ImGui::Text("Pos: %d, %d", ent->getPos().getX(), ent->getPos().getY());
            }
        }

        ImGui::SeparatorText("Attributes");
        for (auto kp : EntityStatViewKeys)
        {
            ImGui::Text("%s: %d", kp.second.c_str(), ent->getAsInt(kp.first));
        }

        ImGui::SeparatorText("Skills");
        ImGui::Text("Active:");
        if (ImGui::BeginListBox("##active_skills", {200, 60}))
        {
            const auto activeSkills = MainRegistry::getInstance()->activeSkills;

            for (auto p : ent->getActiveSkillCD())
            {
                auto skillData = activeSkills->get(p.first);
                auto entryID = uuids::to_string(ent->uuid) + "_" + skillData.id;
                auto display = skillData.name + " - CD: " + std::to_string(p.second);
                ImGui::PushID(entryID.c_str());
                if (ImGui::Selectable(display.c_str()))
                {
                    std::cout << "clicked on " << entryID << std::endl;
                }
                ImGui::SetItemTooltip("%s", skillData.description.c_str());
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
        ImGui::Text("Passive:");
        if (ImGui::BeginListBox("##passive_skills", {200, 60}))
        {
            const auto passiveSkills = MainRegistry::getInstance()->passiveSkills;

            for (auto p : ent->getPassiveSkillCD())
            {
                auto skillData = passiveSkills->get(p.first);
                auto entryID = uuids::to_string(ent->uuid) + "_" + skillData.id;
                auto display = skillData.name + " - CD: " + std::to_string(p.second);
                ImGui::PushID(entryID.c_str());
                if (ImGui::Selectable(display.c_str()))
                {
                    std::cout << "clicked on " << entryID << std::endl;
                }
                ImGui::SetItemTooltip("%s", skillData.description.c_str());
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
        ImGui::SeparatorText("Buffs / Debuffs");
        if (ImGui::BeginListBox("##buffs_view", {200, 60}))
        {
            const auto buffs = MainRegistry::getInstance()->buffTemplates;

            for (auto buff : ent->getBuffs())
            {
                auto buffData = buffs->get(buff.id);
                auto entryID = uuids::to_string(ent->uuid) + "_" + buffData.id;
                auto display = buffData.name + " - T: " + std::to_string(buff.getTurns());
                ImGui::PushID(entryID.c_str());
                if (ImGui::Selectable(display.c_str()))
                {
                    std::cout << "clicked on " << entryID << std::endl;
                }
                ImGui::SetItemTooltip("%s", buffData.description.c_str());
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        ImGui::SeparatorText("Equipments");
        auto equipList = MainRegistry::getInstance()->equipmentTemplates;
        ImGui::BeginDisabled(inCombat || gameMgr->getExploreState() == ExploreState::BeginRound || gameMgr->getExploreState() == ExploreState::EndRound || gameMgr->getCurrentPlayerUUID() != ent->uuid);
        for (auto p : ent->getEquipments())
        {
            auto id = uuids::to_string(ent->uuid) + "_" + equipmentTexts.at(p.first);
            auto popupID = "equip";
            ImGui::Text("%s:", equipmentTexts.at(p.first).c_str());
            ImGui::SameLine();
            if (p.second.has_value())
            {
                ImGui::Text(equipList->get(p.second->id).name.c_str());
                ImGui::SetItemTooltip(equipList->get(p.second->id).description.c_str());
                ImGui::SameLine();
                ImGui::PushID(id.c_str());
                if (ImGui::Button("Unequip"))
                {
                    gameMgr->getInventory()->addEquipment(ent->removeEquipment(p.first));
                }
                ImGui::PopID();
            }
            else
            {
                ImGui::Text("None");
                ImGui::SameLine();
                ImGui::PushID(id.c_str());
                if (ImGui::Button("Equip"))
                {
                }
                ImGui::PopID();
                if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
                {
                    ImGui::Text("Select one to equip:");
                    if (ImGui::BeginListBox("##equip_select", {200, 100}))
                    {
                        for (auto e : gameMgr->getInventory()->getEquipments())
                        {
                            if (equipList->get(e.id).equipmentType == p.first)
                            {
                                ImGui::PushID((id + uuids::to_string(e.uuid)).c_str());
                                if (ImGui::Selectable(equipList->get(e.id).name.c_str(), false))
                                {
                                    ent->addEquipment(gameMgr->getInventory()->removeEquipment(e.uuid));
                                    ImGui::CloseCurrentPopup();
                                }
                                ImGui::SetItemTooltip(equipList->get(e.id).name.c_str());
                                ImGui::PopID();
                            }
                        }
                        ImGui::EndListBox();
                    }
                    ImGui::EndPopup();
                }
            }
        }
        ImGui::EndDisabled();

        if (ent->isPlayer() && !inCombat)
        {
            ImGui::Separator();
            ImGui::PushID(("goto_" + uuids::to_string(ent->uuid)).c_str());
            if (ImGui::Button("Go to"))
            {
                double cx = ent->getPos().getX(), cy = ent->getPos().getY();
                gameViewFocus = ImVec2(cx * 60, cy * 60);
                shouldFocus = true;
            }
            ImGui::PopID();
        }
        ImGui::EndDisabled();

        ImGui::End();
    }

    void ViewManager::renderGameStateControl(bool disabled)
    {
        static const std::map<GameState, std::string> gameStateTexts{
            {GameState::None, "IN ABYSS"},
            {GameState::Explore, "Exploring"},
            {GameState::Interact, "In interactions"},
            {GameState::Teleport, "Casting teleportation magic"},
            {GameState::Shop, "Shopping"},
            {GameState::Battle, "In battle"}};

        static const std::map<ExploreState, std::string> exploreStateTexts{
            {ExploreState::None, "Questioning the world"},
            {ExploreState::BeginRound, "Questioning existence"},
            {ExploreState::BeginTurn, "Preparing"},
            {ExploreState::RollAP, "Rolling AP"},
            {ExploreState::Move, "Moving around"},
            {ExploreState::EndTurn, "Waiting to end turn"},
            {ExploreState::EndRound, "Reminiscing past choices"}};

        auto gameMgr = GameManager::getInstance();
        auto world = gameMgr->getWorld();

        ImGui::Begin("Game Status", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs * disabled);

        ImGui::BeginDisabled(disabled);

        ImGui::Text("Round: %lld", gameMgr->getRoundNumber());
        ImGui::Text("Currently: %s", gameStateTexts.at(gameMgr->getGameState()).c_str());
        ImGui::Text("Turn order");
        if (ImGui::BeginListBox("##ExploreTurnOrder", {0, 120}))
        {
            auto order = gameMgr->getPlayerTurnOrder();
            for (size_t i = 0; i < order.size(); i++)
            {
                ImGui::BeginDisabled(world->getPlayerByUUID(order[i])->isDead());
                ImGui::Selectable(world->getPlayerByUUID(order[i])->name.c_str(), gameMgr->getCurrentPlayerIndex() == i);
                ImGui::EndDisabled();
            }
            ImGui::EndListBox();
        }
        ImGui::Text("Player is currently: %s", exploreStateTexts.at(gameMgr->getExploreState()).c_str());

        ImGui::Separator();
        ImGui::BeginDisabled(gameMgr->getExploreState() != ExploreState::BeginRound);
        if (ImGui::Button("Begin Round"))
        {
            gameMgr->beginRound();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(gameMgr->getExploreState() != ExploreState::BeginTurn);
        if (ImGui::Button("Roll AP"))
        {
            gameMgr->beginTurn();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(gameMgr->getExploreState() != ExploreState::EndTurn);
        if (ImGui::Button("Next Turn"))
        {
            gameMgr->endTurn();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(gameMgr->getExploreState() != ExploreState::EndRound);
        if (ImGui::Button("Next Round"))
        {
            gameMgr->endRound();
        }
        ImGui::EndDisabled();

        ImGui::EndDisabled();

        ImGui::End();
    }

    void ViewManager::renderInventoryControl(bool disabled)
    {
        auto gameMgr = GameManager::getInstance();
        auto inv = gameMgr->getInventory();
        auto itemEntries = MainRegistry::getInstance()->itemTemplates;
        auto equipEntries = MainRegistry::getInstance()->equipmentTemplates;

        ImGui::Begin("Inventory", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::BeginDisabled(disabled);

        ImGui::Text("Gold: ");
        ImGui::SameLine();
        ImGui::TextColored({1, 1, 0, 1}, "%d", inv->getGold());
        ImGui::SeparatorText("Items");
        if (ImGui::BeginListBox("##inventory_items", {200, 100}))
        {
            for (auto item : inv->getItems())
            {
                auto itemData = itemEntries->get(item.id);
                auto display = itemData.name + " x " + std::to_string(item.amount);
                if (ImGui::Selectable(display.c_str(), false))
                {
                    gameMgr->useItem(item.id);
                }
                ImGui::SetItemTooltip(itemData.description.c_str());
            }
            ImGui::EndListBox();
        }

        ImGui::SeparatorText("Equipments");
        if (ImGui::BeginListBox("##inventory_equipments", {200, 100}))
        {
            for (auto equip : inv->getEquipments())
            {
                auto equipData = equipEntries->get(equip.id);
                auto display = equipData.name;
                if (ImGui::Selectable(display.c_str(), false))
                {
                }
                ImGui::SetItemTooltip("%s\n%s", equipmentTexts.at(equipData.equipmentType).c_str(), equipData.description.c_str());
            }
            ImGui::EndListBox();
        }

        ImGui::EndDisabled();

        ImGui::End();
    }

    void ViewManager::renderExploring()
    {
        auto gameMgr = GameManager::getInstance();
        switch (gameMgr->getGameState())
        {
        case GameState::Explore:
            renderMovingPhase();
        case GameState::Teleport:
        case GameState::Interact:
            for (auto ep : gameMgr->getWorld()->getPlayers())
                renderEntityInfo(ep);
            break;
        default:
            break;
        }
    }

    void ViewManager::renderMovingPhase()
    {
        auto gameMgr = GameManager::getInstance();

        if (gameMgr->getExploreState() == ExploreState::RollAP)
        {
            size_t rollResult = 0;
            bool finishedRolling = false;
            auto ep = gameMgr->getCurrentPlayer();
            gameMgr->prepAPRoll();
            showDiceRollPopup("Roll AP", finishedRolling, rollResult, ep, ep->getAsInt("max_ap"), ep->getAPChance(), ep->getAsInt("ap_boost"));
            if (finishedRolling)
                gameMgr->markAPRolled(rollResult);
        }

        if (invalidPos)
        {
            ImGui::OpenPopup("Invalid position");
            ImGUI_CenterNextWindow();
        }
        if (ImGui::BeginPopupModal("Invalid position", &invalidPos, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextColored({1, 0, 0, 1}, "You can not go there now");
            if (ImGui::Button("Fine"))
            {
                invalidPos = false;
            }
            ImGui::EndPopup();
        }
    }

    void ViewManager::renderInteractions()
    {
        auto gameMgr = GameManager::getInstance();
        auto pos = gameMgr->getCurrentPlayer()->getPos();
        switch (gameMgr->getInteractableType(pos))
        {
        case InteractableType::Enemy:
            showInteractionPopup(InteractionType::Enemy, doneInteration, pos);
            break;
        case InteractableType::RE:
            if (auto re = gameMgr->getWorld()->getRectEntityAt(pos))
            {
                if (re->type == RectEntityType::Shop)
                    showInteractionPopup(InteractionType::Shop, doneInteration, pos);
            }
            break;
        default:
            break;
        }
    }

    void ViewManager::renderEnemyInteraction(const Vec2i &pos)
    {
        static bool rollingForAmbush = false;
        static bool rollingForSneak = false;
        static bool ambushFailed = false;
        static bool doAmbush = false;
        static bool sneakFailed = false;
        static bool doSneak = false;

        auto gameMgr = GameManager::getInstance();
        auto ep = gameMgr->getCurrentPlayer();

        ImGui::Text("You found an enemy: %s", gameMgr->getWorld()->getEntitiesAt(pos).front()->name.c_str());
        if (doAmbush)
            ImGui::TextColored({0, 1, 0, 1}, "Ambush succeed!");
        if (ambushFailed)
            ImGui::TextColored({1, 0, 0, 1}, "Ambush failed...");
        if (sneakFailed)
            ImGui::TextColored({1, 0, 0, 1}, "Sneak failed...");
        ImGui::Separator();
        if (ImGui::Button("Attack"))
        {
            gameMgr->triggerBattle(pos, doAmbush, ambushFailed);
            doAmbush = false;
            ambushFailed = false;
            doSneak = false;
            sneakFailed = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(doAmbush || ambushFailed || sneakFailed);
        if (ImGui::Button("Ambush"))
        {
            rollingForAmbush = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Sneak"))
        {
            rollingForSneak = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Retreat"))
        {
            gameMgr->markInteractionDone(true);
            gameMgr->retreatPlayer();
            doAmbush = false;
            ambushFailed = false;
            doSneak = false;
            sneakFailed = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();
        if (rollingForAmbush)
        {
            size_t ambushRoll;
            bool finishedRolling = false;
            showDiceRollPopup("Roll for Ambush", finishedRolling, ambushRoll, ep, 3, ep->get("speed") / 100);
            if (finishedRolling)
            {
                doAmbush = ambushRoll == 3;
                ambushFailed = ambushRoll != 3;
                rollingForAmbush = false;
            }
        }
        if (rollingForSneak)
        {
            size_t sneakRoll;
            bool finishedRolling = false;
            showDiceRollPopup("Roll for Sneak", finishedRolling, sneakRoll, ep, 3, ep->get("speed") / 100);
            if (finishedRolling)
            {
                doSneak = sneakRoll == 3;
                sneakFailed = sneakRoll != 3;
                rollingForSneak = false;
            }
        }
        if (doSneak)
        {
            gameMgr->markInteractionDone();
            doAmbush = false;
            ambushFailed = false;
            doSneak = false;
            sneakFailed = false;
            ImGui::CloseCurrentPopup();
        }
    }

    void ViewManager::renderShopInteraction(const Vec2i &pos)
    {
        auto gameMgr = GameManager::getInstance();
        auto ep = gameMgr->getCurrentPlayer();

        ImGui::Text("You found a shop. You wonder what it can offers...");
        ImGui::Separator();
        if (ImGui::Button("Enter"))
        {
            gameMgr->enterShop();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Ignore"))
        {
            gameMgr->markInteractionDone();
            ImGui::CloseCurrentPopup();
        }
    }

    void ViewManager::renderCombatSystemControl()
    {
        static const std::map<CombatState, std::string> combatStateTexts{
            {CombatState::None, "Not in combat"},
            {CombatState::BeginRound, "Planing"},
            {CombatState::BeginTurn, "Preparing"},
            {CombatState::ChooseAction, "Making choices"},
            {CombatState::RollDice, "Rolling dice"},
            {CombatState::ResolveActions, "Predicting"},
            {CombatState::ProcessActions, "Realizing"},
            {CombatState::EndTurn, "Cleaning weapons"},
            {CombatState::EndRound, "Regroup"},
            {CombatState::EndBattle, "Cleanup crime scene"}};

        ImGui::Begin("Combat Status", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        auto combatSys = CombatSystem::getInstance();

        ImGui::Text("Round: %lld", combatSys->getRoundNumber());
        ImGui::SameLine();
        ImGui::Text("Turn: %lld", combatSys->getTurnNumber());
        ImGui::Text("Currently: %s", combatStateTexts.at(combatSys->getCombatState()).c_str());
        ImGui::BeginGroup();
        ImGui::Text("Turn order");
        if (ImGui::BeginListBox("##CombatTurnOrder", {200, 120}))
        {
            auto priorities = combatSys->getPriorities();
            auto entities = combatSys->getEntities();
            auto apCount = combatSys->getActionPerformed();
            for (auto uuid : priorities)
            {
                auto ent = [entities](uuids::uuid uuid) -> std::shared_ptr<Entity>
                {
                    for (auto e : entities)
                        if (e->uuid == uuid)
                            return e;
                    return nullptr;
                }(uuid);
                auto display = ent->name + " - AP: " + std::to_string(apCount[uuid]);
                if (ImGui::Selectable(display.c_str(), uuid == priorities.front()))
                {
                }
            }
            ImGui::EndListBox();
        }
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::BeginDisabled(combatSys->getCombatState() != CombatState::ChooseAction || !combatSys->getCurrentEntity()->isPlayer());
        ImGui::BeginGroup();
        ImGui::BeginDisabled(combatSys->getActionSelectionType() == ActionSelectionType::Skill);
        ImGui::PushID("combat_control_select_skill_type");
        if (ImGui::Button("Skill"))
        {
            combatSys->setActionSelectionType(ActionSelectionType::Skill);
        }
        ImGui::PopID();
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(combatSys->getActionSelectionType() == ActionSelectionType::Item);
        ImGui::PushID("combat_control_select_skill_type");
        if (ImGui::Button("Item"))
        {
            combatSys->setActionSelectionType(ActionSelectionType::Item);
        }
        ImGui::PopID();
        ImGui::EndDisabled();
        if (ImGui::BeginListBox("##action_select", {200, 120}))
        {
            if (combatSys->getActionSelectionType() == ActionSelectionType::Skill)
            {

                const auto activeSkills = MainRegistry::getInstance()->activeSkills;

                auto ent = combatSys->getCurrentEntity();
                for (auto p : combatSys->getActionCandidates())
                {
                    if (ImGui::Selectable(activeSkills->get(p).name.c_str(), p == combatSys->getSelectedSctionID()))
                    {
                        if (p != combatSys->getSelectedSctionID())
                            combatSys->markActionSelected(p);
                    }
                    ImGui::SetItemTooltip("%s", activeSkills->get(p).description.c_str());
                }
            }
            else if (combatSys->getActionSelectionType() == ActionSelectionType::Item)
            {
                const auto itemData = MainRegistry::getInstance()->itemTemplates;
                for (auto p : combatSys->getActionCandidates())
                {
                    if (ImGui::Selectable(itemData->get(p).name.c_str(), p == combatSys->getSelectedSctionID()))
                    {
                        if (p != combatSys->getSelectedSctionID())
                            combatSys->markActionSelected(p);
                    }
                    ImGui::SetItemTooltip(itemData->get(p).description.c_str());
                }
            }
            ImGui::EndListBox();
        }
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::Text("Select Target");
        if (ImGui::BeginListBox("##target_select", {200, 120}))
        {
            for (auto e : combatSys->getTargetCandidates())
            {
                if (ImGui::Selectable(e->name.c_str(), e->uuid == combatSys->getSelectedTarget()))
                {
                    combatSys->markTargetSelected(e->uuid);
                }
            }
            ImGui::EndListBox();
        }
        ImGui::EndGroup();
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::BeginDisabled(combatSys->getCombatState() != CombatState::BeginRound);
        if (ImGui::Button("Begin Round"))
        {
            combatSys->beginRound();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(combatSys->getCombatState() != CombatState::BeginTurn);
        if (ImGui::Button("Begin Turn"))
        {
            combatSys->beginTurn();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(combatSys->getCombatState() != CombatState::ChooseAction || !combatSys->readyToRollDice());
        if (ImGui::Button("Confirm choices"))
        {
            combatSys->confirmChoice();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(combatSys->getCombatState() != CombatState::ResolveActions);
        if (ImGui::Button("Resolve Actions"))
        {
            combatSys->resolveActions();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(combatSys->getCombatState() != CombatState::EndTurn);
        if (ImGui::Button("Next Turn"))
        {
            combatSys->endTurn();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(combatSys->getCombatState() != CombatState::EndRound);
        if (ImGui::Button("Next Round"))
        {
            combatSys->endRound();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(combatSys->getCombatState() != CombatState::EndBattle);
        if (ImGui::Button("End Battle"))
        {
            combatSys->endBattle();
        }
        ImGui::EndDisabled();

        ImGui::End();
    }

    void ViewManager::renderCombat()
    {
        auto combatSys = CombatSystem::getInstance();

        for (auto ep : combatSys->getPlayers())
            renderEntityInfo(ep, true);
        for (auto en : combatSys->getEnemies())
            renderEntityInfo(en, true);
        renderCombatSystemControl();

        if (combatSys->getCombatState() == CombatState::RollDice)
        {
            if (combatSys->getCurrentEntity()->isPlayer())
            {
                if (combatSys->getActionSelectionType() == ActionSelectionType::Skill)
                {
                    static double rollChance = -1;
                    size_t rollResult = 0;
                    bool finishedRolling = false;
                    auto ep = std::dynamic_pointer_cast<Player>(combatSys->getCurrentEntity());
                    auto skillData = MainRegistry::getInstance()->activeSkills->get(combatSys->getSelectedSctionID());
                    if (rollChance == -1)
                        rollChance = skillData.rollChanceExpr.eval(ep->getMathContext("self")) / 100;
                    auto diceRolls = skillData.diceRolls;
                    if (skillData.id == "active:basic_attack")
                        diceRolls = combatSys->getCurrentEntity()->getWeaponDiceRoll();
                    showDiceRollPopup("Roll Skill", finishedRolling, rollResult, ep, diceRolls, rollChance);
                    if (finishedRolling)
                    {
                        rollChance = -1;
                        combatSys->markDiceRolled(rollResult);
                    }
                }
                else if (combatSys->getActionSelectionType() == ActionSelectionType::Item)
                {
                    combatSys->markDiceRolled(0);
                }
            }
        }
    }

    void ViewManager::renderShop(const std::shared_ptr<ShopRectEntity> &shop)
    {
        auto gameMgr = GameManager::getInstance();
        auto inv = gameMgr->getInventory();
        ImGUI_CenterNextWindow();
        ImGui::PushID(uuids::to_string(shop->uuid).c_str());
        ImGui::Begin(shop->name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Click to buy!");

        ImGui::BeginGroup();
        ImGui::Text("Items:");
        if (ImGui::BeginListBox("##shop_items", {300, 0}))
        {
            auto itemList = MainRegistry::getInstance()->itemTemplates;
            for (auto item : shop->getInventory()->getItems())
            {
                auto itemData = itemList->get(item.id);
                auto id = "shop_buy_" + item.id;
                auto display = std::to_string(item.amount) + " x " + itemData.name + " : " + std::to_string(itemData.shopValue) + "G";
                ImGui::BeginDisabled(inv->getGold() < itemData.shopValue);
                ImGui::PushID(id.c_str());
                if (ImGui::Selectable(display.c_str(), false))
                {
                    shop->buyItem(item.id, itemData.shopValue);
                }
                ImGui::SetItemTooltip(itemData.description.c_str());
                ImGui::PopID();
                ImGui::EndDisabled();
            }
            ImGui::EndListBox();
        }
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Equipments:");
        if (ImGui::BeginListBox("##shop_equips", {300, 0}))
        {
            auto equipList = MainRegistry::getInstance()->equipmentTemplates;
            for (auto equip : shop->getInventory()->getEquipments())
            {
                auto equipData = equipList->get(equip.id);
                auto id = "shop_buy_" + equip.id + "_" + uuids::to_string(equip.uuid);
                auto display = equipData.name + " : " + std::to_string(equipData.shopValue) + "G";
                ImGui::BeginDisabled(inv->getGold() < equipData.shopValue);
                ImGui::PushID(id.c_str());
                if (ImGui::Selectable(display.c_str(), false))
                {
                    shop->buyEquipment(equip.uuid, equipData.shopValue);
                }
                ImGui::SetItemTooltip("%s\n%s", equipmentTexts.at(equipData.equipmentType).c_str(), equipData.description.c_str());
                ImGui::PopID();
                ImGui::EndDisabled();
            }
            ImGui::EndListBox();
        }
        ImGui::EndGroup();

        if (ImGui::Button("Exit Shop"))
        {
            gameMgr->exitShop();
        }
        ImGui::End();
        ImGui::PopID();
    }

    void ViewManager::renderShop()
    {
        auto gameMgr = GameManager::getInstance();
        auto re = gameMgr->getWorld()->getRectEntityAt(gameMgr->getCurrentPlayer()->getPos());
        if (re->type == RectEntityType::Shop)
            renderShop(std::static_pointer_cast<ShopRectEntity>(re));
    }

    void ViewManager::renderTeleportControl()
    {
        ImGui::Begin("Teleport Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("Cancel Teleport"))
        {
            GameManager::getInstance()->endTeleport();
        }
        ImGui::End();
    }

    void ViewManager::renderMainControl()
    {
        ImGui::Begin("Main Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("Save game"))
        {
            fileDialog.OpenDialog("SaveMapKey", "Save Map", ".json", config);
        }
        if (ImGui::Button("Exit to main menu without saving"))
        {
            viewState = ViewState::MainMenu;
            GameManager::getInstance()->reset();
        }
        if (ImGui::Button("Save and exit to main menu"))
        {
            fileDialog.OpenDialog("SaveMapAndExitKey", "Save Map", ".json", config);
        }
        ImGui::End();

        if (fileDialog.Display("SaveMapKey"))
        {
            if (fileDialog.IsOk())
            {
                auto saveFile = fileDialog.GetFilePathName();
                GameManager::getInstance()->saveMap(saveFile);
                std::cout << "Game saved to: " << saveFile << std::endl;
            }
            fileDialog.Close();
        }
        if (fileDialog.Display("SaveMapAndExitKey"))
        {
            if (fileDialog.IsOk())
            {
                auto saveFile = fileDialog.GetFilePathName();
                GameManager::getInstance()->saveMap(saveFile);
                std::cout << "Game saved to: " << saveFile << std::endl;
                viewState = ViewState::MainMenu;
                GameManager::getInstance()->reset();
            }
            fileDialog.Close();
        }
    }

    void ViewManager::renderMainMenu()
    {
        ImGUI_FullscreenNextWindow();
        ImGui::Begin("##main_menu", nullptr, ImGUI_Flags_FullscreenWindow);

        ImGUI_AlignForWidth(ImGui::CalcTextSize("For the K!").x);
        ImGui::SetCursorPosY(250);
        ImGui::Text("For the K!");

        ImGUI_AlignForWidth(150);
        if (ImGui::Button("New game", {150, 0}))
        {
            viewState = ViewState::NewGame;
        }

        ImGUI_AlignForWidth(150);
        if (ImGui::Button("Load game", {150, 0}))
        {
            fileDialog.OpenDialog("LoadMapKey", "Load Map", ".json", config);
        }

        ImGUI_AlignForWidth(150);
        if (ImGui::Button("Load Demo", {150, 0}))
        {
            GameManager::getInstance()->loadMap("assets/gamedata/demo_map.json");
            viewState = ViewState::InGame;
        }

        ImGui::End();

        if (fileDialog.Display("LoadMapKey"))
        {
            if (fileDialog.IsOk())
            {
                auto saveFile = fileDialog.GetFilePathName(2);
                if (std::filesystem::exists(saveFile))
                {
                    GameManager::getInstance()->loadMap(saveFile);
                    std::cout << "Game loaded from: " << saveFile << std::endl;
                    viewState = ViewState::InGame;
                }
            }
            fileDialog.Close();
        }
    }

    void ViewManager::renderNewGame()
    {
    }

    void ViewManager::renderInGame()
    {
        auto gameMgr = GameManager::getInstance();
        renderMap(gameMgr->getGameState() == GameState::Battle || gameMgr->getGameState() == GameState::Shop);
        renderGameStateControl(gameMgr->getGameState() == GameState::Battle || gameMgr->getGameState() == GameState::Shop);
        renderInventoryControl(gameMgr->getGameState() == GameState::Battle || gameMgr->getGameState() == GameState::Shop);
        renderExploring();
        if (gameMgr->getGameState() == GameState::Interact)
            renderInteractions();
        if (gameMgr->getGameState() == GameState::Battle)
            renderCombat();
        if (gameMgr->getGameState() == GameState::Shop)
            renderShop();
        if (gameMgr->getGameState() == GameState::Teleport)
            renderTeleportControl();
        renderMainControl();
    }

    void ViewManager::showDiceRollPopup(const std::string &popupName, bool &finishedRolling, size_t &rollResult, const std::shared_ptr<Player> &ep, size_t rollAmount, double rollChance, int guarentee)
    {
        ImGui::OpenPopup(popupName.c_str());
        ImGUI_CenterNextWindow();
        if (ImGui::BeginPopupModal(popupName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            for (int i = 0; i < rollAmount; i++)
            {
                if (i)
                    ImGui::SameLine();
                ImVec4 color = {0.5, 0.5, 0.5, 1};
                if (doneRolling)
                {
                    if (i < diceRollRes)
                        color = {0, 1, 0, 1};
                    else
                        color = {1, 0, 0, 1};
                }
                else
                {
                    if (i < guarentee + focusUsed)
                        color = {0, 1, 0, 1};
                    if (i < -(guarentee + focusUsed))
                        color = {1, 0, 0, 1};
                }
                ImGui::TextColored(color, "*");
            }

            ImGui::BeginDisabled(doneRolling);
            ImGui::SliderInt("Use Focus", &focusUsed, 0, std::min((int)rollAmount - guarentee, ep->getAsInt("focus") + doneRolling * focusUsed), "%d", ImGuiSliderFlags_NoInput);
            ImGui::EndDisabled();

            ImGui::Separator();
            if (!doneRolling)
            {
                if (ImGui::Button("Roll"))
                {
                    guarentee += focusUsed;
                    diceRollRes = Dice::rollUniformDices(rollAmount, std::max(0.0, std::min(1.0, rollChance * ep->get("dice_chance_mult"))), guarentee);
                    doneRolling = true;
                    ep->dec("focus", focusUsed);
                }
            }
            else
            {
                if (ImGui::Button("Confirm"))
                {
                    doneRolling = false;
                    finishedRolling = true;
                    rollResult = diceRollRes;
                    diceRollRes = 0;
                    focusUsed = 0;
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }

    void ViewManager::showInteractionPopup(InteractionType interactionType, bool &finishedInteraction, const Vec2i &pos)
    {
        static const std::map<InteractionType, std::string> PopupTitles{
            {InteractionType::Enemy, "Encountered enemies"},
            {InteractionType::Shop, "Encountered a shop"}};

        ImGui::OpenPopup(PopupTitles.at(interactionType).c_str());
        ImGUI_CenterNextWindow();
        if (ImGui::BeginPopupModal(PopupTitles.at(interactionType).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            switch (interactionType)
            {
            case InteractionType::Enemy:
                renderEnemyInteraction(pos);
                break;
            case InteractionType::Shop:
                renderShopInteraction(pos);
                break;
            default:
                break;
            }

            ImGui::EndPopup();
        }
    }
} // namespace FTK::GUI