#pragma once
#include <ranges>
#include <algorithm>

namespace utl {
namespace ranges = std::ranges;

struct min_element_val_fn {
    template <
        ranges::forward_range R,
        typename T,
        class Proj = std::identity,
        std::indirect_strict_weak_order<std::projected<ranges::iterator_t<R>, Proj>> Comp = ranges::less>
    constexpr T operator()(R&& r, T init = {}, Comp comp = {}, Proj proj = {}) const {
        auto outputIt = ranges::min_element(r, comp, proj);
        if (outputIt == ranges::end(r))
            return init;
        return std::min(std::invoke(proj, *outputIt), init);
    }
};
inline constexpr min_element_val_fn min_element_val;
}  // namespace utl
