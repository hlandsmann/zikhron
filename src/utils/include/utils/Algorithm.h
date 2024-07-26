#pragma once
#include <algorithm>
#include <initializer_list>

namespace utl {

template<class T>
auto isEither(T value, std::initializer_list<T> values) -> bool
{
    return std::ranges::any_of(values, [value](const auto& item) {
        return item == value;
    });
}
template<class T>
auto isNeither(T value, std::initializer_list<T> values) -> bool
{
    return std::ranges::none_of(values, [value](const auto& item) {
        return item == value;
    });
}

} // namespace utl
