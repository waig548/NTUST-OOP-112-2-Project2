
#define WIDTH 140
#define HEIGHT 50

#include <conio.h>
#include <iostream>
#include <fstream>

#include "Entity.h"

#include "Expr.h"

#include "Vec.h"

#include "World.h"
// #define FTK_SERIALIZER_IMPL
#include "Serializer.h"

#include "cli_util.h"

#include "Registry.h"

#include "GameManager.h"

enum ValidKeys
{
    KEY_N = 'N',
    KEY_L = 'L',
    KEY_W = 'W',
    KEY_A = 'A',
    KEY_S = 'S',
    KEY_D = 'D',
    KEY_Q = 'Q',
    KEY_E = 'E',

    KEY_UP = 72,
    KEY_LEFT = 75,
    KEY_DOWN = 80,
    KEY_RIGHT = 77,
    KEY_ESC = 27
};

void movePlayer(const std::shared_ptr<FTK::Player> &player, const FTK::Vec2i &diff, const FTK::Vec2i &TL, const FTK::Vec2i &BR)
{
    auto targetPos = player->getPos() + diff;
    if (targetPos.getX() < TL.getX() || targetPos.getX() >= BR.getX() || targetPos.getY() < TL.getY() || targetPos.getY() >= BR.getY())
        return;
    player->setPos(targetPos);
}

void renderScreen(const std::shared_ptr<FTK::Player> &player)
{
    FTK::CLI::clearScreen();
    FTK::CLI::setCursorPosition(player->getPos().getX(), player->getPos().getY());
    std::cout << "1";
}

int main(int argc, char **argv, char **envp)
{
    FTK::GameManager::getInstance()->loadMap("../../../demo_map.json");

    auto world = FTK::GameManager::getInstance()->getWorld();
    // world->removeEntity(uuids::uuid_system_generator{}());
    nlohmann::ordered_json nj = world;
    std::cout << std::setw(4) << nj << std::setw(0) << std::endl;

    // world = nj;

    // FTK::MainRegistry::getInstance();



    while (!FTK::CLI::checkConsoleSize(WIDTH, HEIGHT))
        FTK::CLI::printConsoleSizeCheckScreen();

    int w, h;
    FTK::CLI::clearScreen();
    FTK::CLI::getConsoleSize(w, h);
    std::cout << "running in " << w << " x " << h << std::endl;

    std::shared_ptr<FTK::Player> aa = FTK::GameManager::getInstance()->getWorld()->getPlayerByIndex(0);
    // FTK::World world({140, 50});

    while (true)
    {
        int ch = _getch();
        ch = std::toupper(ch);
        if (ch == KEY_ESC)
            break;
        if (ch == KEY_W)
            movePlayer(aa, {0, -1}, {0, 0}, {w, h});
        if (ch == KEY_A)
            movePlayer(aa, {-1, 0}, {0, 0}, {w, h});
        if (ch == KEY_S)
            movePlayer(aa, {0, 1}, {0, 0}, {w, h});
        if (ch == KEY_D)
            movePlayer(aa, {1, 0}, {0, 0}, {w, h});
        renderScreen(aa);
    }
}