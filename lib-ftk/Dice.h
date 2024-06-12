#ifndef FTK_DICE_H
#define FTK_DICE_H

#include <memory>

#include "utils.h"

namespace FTK
{
    class Dice
    {
    public:
        Dice();
        Dice(double rollChance);
        Dice(const Dice &other);
        ~Dice() = default;

        bool operator()() const;

        bool roll() const;

        void markAlwaysSuccess();
        void markAlwaysFail();

        double rollChance;

        static size_t rollUniformDices(size_t amount, double rollChance, int guarentee = 0);
    };
} // namespace FTK

#endif // FTK_DICE_H
