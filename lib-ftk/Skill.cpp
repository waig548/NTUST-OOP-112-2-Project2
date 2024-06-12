#include "Skill.h"

namespace FTK
{
    Math::Context ActiveSkill::getContext(const std::string &key, const Math::Context &base)
    {
        auto res = base;
        res[key] = Math::Context();
        res[key]["skill_type"] = (int)skillType;
        res[key]["target_type"] = (int)targetType;
        res[key]["dice_rolls"] = diceRolls;
        res[key]["allow_partial"] = allowPartial;
        res["dice_rolls"] = diceRolls;
        return res;
    }

} // namespace FTK
