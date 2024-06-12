#ifndef FTK_GUI_GUI_H
#define FTK_GUI_GUI_H

#include <imgui.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <string>

namespace FTK::GUI
{
    typedef struct
    {
        GLuint textureID;
        int width;
        int height;
    } Texture;

    const auto ImGUI_Flags_FullscreenWindow = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;

    void ImGUI_FullscreenNextWindow();
    void ImGUI_CenterNextWindow();

    void ImGUI_AlignForWidth(float width, float alignment = 0.5f);

    GLFWwindow *createWindow(const char *title, int width, int height);
    bool windowShouldClose(GLFWwindow *window);
    void destroyWindow(GLFWwindow *window);
    void beginUI();
    void endUI();
    void loadTextures();
    Texture getTextureByID(const std::string &id);
    Texture getRectTexture(const std::string &id, int metadata);
    Texture getRectIconTexture(const std::string& type);
} // namespace FTK::GUI

#endif // FTK_GUI_GUI_H
