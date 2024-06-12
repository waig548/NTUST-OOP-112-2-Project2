#ifndef FTK_GUI_VIEW_H
#define FTK_GUI_VIEW_H

#include <memory>
#include <string>

#include "Entity.h"
#include "Rect.h"

namespace FTK::GUI
{
    class ViewManager
    {
    public:
        void render();

        static std::shared_ptr<ViewManager> getInstance();

    private:
        enum class ViewState
        {
            MainMenu,
            NewGame,
            InGame
        };

        enum class InteractionType
        {
            Enemy,
            Shop,
            Chest,
            Tent,
            Event
        };
        ViewManager();

        void renderMap(bool disabled = false);
        void renderEntityInfo(std::shared_ptr<Entity> ent, bool inCombat = false);
        void renderGameStateControl(bool disabled = false);
        void renderInventoryControl(bool disabled = false);

        void renderExploring();
        void renderMovingPhase();

        void renderInteractions();
        void renderEnemyInteraction(const Vec2i &pos);
        void renderShopInteraction(const Vec2i &pos);

        void renderCombatSystemControl();
        void renderCombat();

        void renderShop(const std::shared_ptr<ShopRectEntity> &shop);
        void renderShop();

        void renderTeleportControl();

        void renderMainControl();

        void renderMainMenu();
        void renderNewGame();
        void renderInGame();

        void showDiceRollPopup(const std::string &popupName, bool &finishedRolling, size_t &rollResult, const std::shared_ptr<Player> &ep, size_t rollAmount, double rollChance, int guarentee = 0);
        void showInteractionPopup(InteractionType interactionType, bool &finishedInteraction, const Vec2i &pos);

        ViewState viewState = ViewState::MainMenu;
    };
} // namespace FTK::GUI

#endif // FTK_GUI_VIEW_H
