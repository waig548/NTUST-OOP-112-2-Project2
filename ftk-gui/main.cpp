

#include "Registry.h"
#include "GameManager.h"

#include "gui.h"
#include "view.h"

int main()
{
    FTK::MainRegistry::getInstance()->exportAll("exports/configs");

    const auto window = FTK::GUI::createWindow("FTK", 1600, 900);

    glClearColor(0.25, 0.25, 0.25, 1);

    while (!FTK::GUI::windowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        FTK::GUI::beginUI();
        FTK::GUI::ViewManager::getInstance()->render();
        FTK::GUI::endUI();
    }

    FTK::GUI::destroyWindow(window);
    if (FTK::GameManager::getInstance()->getWorld())
    {
        int i = 0;
        while (std::filesystem::exists("saves/autosave_" + std::to_string(i) + ".json"))
            i++;
        FTK::GameManager::getInstance()->saveMap("saves/autosave_" + std::to_string(i) + ".json");
    }
}