#include "cli_util.h"

#include <string>
#include <Windows.h>

#include <color.hpp>

namespace FTK::CLI
{
    bool getConsoleSize(int &width, int &height)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        {
            width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            return true;
        }
        return false;
    }

    bool checkConsoleSize(int requiredWidth, int requiredHeight)
    {
        int consoleWidth, consoleHeight;
        if (getConsoleSize(consoleWidth, consoleHeight))
            return (consoleWidth >= requiredWidth) && (consoleHeight >= requiredHeight);
        return false;
    }

    void printConsoleSizeCheckScreen()
    {
        static int lastWidth = 0, lastHeight = 0;
        int curWidth, curHeight;
        if (getConsoleSize(curWidth, curHeight))
        {
            if (curWidth != lastWidth || curHeight != lastHeight)
            {
                clearScreen();
                setCursorPosition(0, 0);
                std::cout << std::string(curWidth, '=');
                setCursorPosition(0, curHeight - 1);
                std::cout << std::string(curWidth, '=');
                std::string s = "Current size: " + std::to_string(curWidth) + " x " + std::to_string(curHeight);
                setCursorPosition(curWidth / 2 - s.size() / 2, curHeight / 2);
                std::cout << s;
                lastWidth = curWidth;
                lastHeight = curHeight;
            }
        }
    }

    void clearScreen()
    {
        static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD topLeft = {0, 0};

        std::cout.flush();
        if (!GetConsoleScreenBufferInfo(hOut, &csbi))
        {
            // TODO handle failure
            throw std::runtime_error("Failed to get console screen buffer info");
        }
        DWORD length = csbi.dwSize.X * csbi.dwSize.Y;

        DWORD written;
        FillConsoleOutputCharacter(hOut, TEXT(' '), length, topLeft, &written);

        FillConsoleOutputAttribute(hOut, hue::DEFAULT_COLOR, length, topLeft, &written);

        SetConsoleCursorPosition(hOut, topLeft);
    }

    void setCursorPosition(int x, int y)
    {
        static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        std::cout.flush();
        SetConsoleCursorPosition(hOut, {(SHORT)x, (SHORT)y});
    }
} // namespace FTK::CLI
