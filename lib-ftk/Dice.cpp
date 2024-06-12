#include "Dice.h"

#include <effolkronium/random.hpp>

namespace FTK
{
    Dice::Dice() : Dice(0)
    {
    }

    Dice::Dice(double rollChance) : rollChance(rollChance)
    {
    }

    Dice::Dice(const Dice &other) : Dice(other.rollChance)
    {
    }

    bool Dice::operator()() const
    {
        return roll();
    }

    bool Dice::roll() const
    {
        return effolkronium::random_static::get<bool>(rollChance);
    }

    void Dice::markAlwaysSuccess()
    {
        rollChance = 1;
    }

    void Dice::markAlwaysFail()
    {
        rollChance = 0;
    }

    size_t Dice::rollUniformDices(size_t amount, double rollChance, int guarentee)
    {
        std::vector<Dice> dices(amount, Dice(rollChance));
        if (guarentee < 0)
            for (int i = 0; i < -guarentee; i++)
                dices[i].markAlwaysFail();
        else
            for (int i = 0; i < guarentee; i++)
                dices[i].markAlwaysSuccess();

        return count(dices, [](auto d)
                     { return d(); });
    }

} // namespace FTK
