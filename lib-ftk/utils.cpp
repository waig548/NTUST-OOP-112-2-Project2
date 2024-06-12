#include "utils.h"

#include <algorithm>
#include <iterator>

namespace FTK
{
    std::string toLower(const std::string &str)
    {
        std::string res;
        std::transform(str.begin(), str.end(), std::back_inserter(res), std::tolower);
        return res;
    }

    std::string toUpper(const std::string &str)
    {
        std::string res;
        std::transform(str.begin(), str.end(), std::back_inserter(res), std::toupper);
        return res;
    }

} // namespace FTK
