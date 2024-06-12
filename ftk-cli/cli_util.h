#ifndef FTK_CLI_CLI_UTIL_H
#define FTK_CLI_CLI_UTIL_H

#include <iostream>

#include "Vec.h"

namespace FTK::CLI
{
    bool getConsoleSize(int &width, int&height);
    bool checkConsoleSize(int requiredWidth, int requiredHeight);
    void printConsoleSizeCheckScreen();
    void clearScreen();
    void setCursorPosition(int x, int y);

    // bool getIntLargerThan(std::istream &stream, int *out, int minimumValue);
} // namespace FTK::CLI

#endif // FTK_CLI_CLI_UTIL_H