#ifndef FTK_UTILS_H
#define FTK_UTILS_H

#include <string>

namespace FTK
{
    template <typename R, typename T, template <typename T, typename Alloc = std::allocator<T>> class Container, class Func>
    Container<R> map(const Container<T> &container, const Func &transform)
    {
        Container<R> res;
        for (T e : container)
            res.push_back(transform(e));
        return res;
    }

    template <typename T, template <typename T, typename Alloc = std::allocator<T>> class Container, class Func>
    Container<T> map(const Container<T> &container, const Func &transform)
    {
        Container<T> res;
        for (T e : container)
            res.push_back(transform(e));
        return res;
    }

    template <typename T, template <typename T, typename Alloc = std::allocator<T>> class Container, class Func>
    Container<T> filter(const Container<T> &container, const Func &predicate)
    {
        Container<T> res;
        for (T e : container)
            if (predicate(e))
                res.push_back(e);
        return res;
    }

    template <typename T, template <typename T, typename Alloc = std::allocator<T>> class Container, class Func>
    size_t count(const Container<T> &container, const Func &predicate)
    {
        size_t res = 0;
        for (T e : container)
            if (predicate(e))
                res++;
        return res;
    }

    std::string toLower(const std::string &str);
    std::string toUpper(const std::string &str);
} // namespace FTK

#endif // FTK_UTILS_H
