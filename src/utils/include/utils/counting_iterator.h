#include <cstddef>
#include <iterator>

namespace utl {

struct counting_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using difference_type = int;
    using pointer = int*;
    using reference = int&;
    size_t count{};
    int ignore{};
    auto operator++()->counting_iterator& {
        ++count;
        return *this;
    }
    auto operator++(int) ->counting_iterator {
        counting_iterator temp = *this;
        ++*this;
        return temp;
    }
    auto operator*()->int& { return ignore; }
};
}  // namespace utl
