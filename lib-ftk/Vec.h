#ifndef FTK_VEC_H
#define FTK_VEC_H

#include <nlohmann/adl_serializer.hpp>

namespace FTK
{
    template <typename T>
    class Vec
    {
    public:
        constexpr Vec() : x(), y() {}
        constexpr Vec(T x, T y) : x(x), y(y) {}
        constexpr Vec(const Vec &other) : Vec(other.x, other.y) {}
        ~Vec() = default;

        Vec &operator=(const Vec &other)
        {
            x = other.x;
            y = other.y;
            return *this;
        }

        T getX() const
        {
            return x;
        }

        T getY() const
        {
            return y;
        }

        Vec operator+(const Vec &other) const
        {
            return add(*this, other);
        }

        Vec operator-(const Vec &other) const
        {
            return sub(*this, other);
        }

        Vec operator-() const
        {
            return neg(*this);
        }

        bool operator==(const Vec &other) const
        {
            return equals(*this, other);
        }

        bool operator!=(const Vec &other) const
        {
            return !equals(*this, other);
        }

        friend std::ostream &operator<<(std::ostream &os, const Vec &v)
        {
            return os << "(" << v.x << ", " << v.y << ")";
        }

    private:
        T x, y;

        static Vec add(const Vec &a, const Vec &b)
        {
            return Vec(a.x + b.x, a.y + b.y);
        }

        static Vec sub(const Vec &a, const Vec &b)
        {
            return Vec(a.x - b.x, a.y - b.y);
        }

        static Vec neg(const Vec &v)
        {
            return Vec((-1) * v.x, (-1) * v.y);
        }

        static bool equals(const Vec &a, const Vec &b)
        {
            return a.x == b.x && a.y == b.y;
        }

        friend nlohmann::adl_serializer<Vec>;
    };

    using Vec2i = Vec<int>;
    using Vec2d = Vec<double>;
} // namespace FTK

#endif // FTK_VEC_H
