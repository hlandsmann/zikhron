#include <stddef.h>
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
    counting_iterator& operator++() {
        ++count;
        return *this;
    }
    counting_iterator operator++(int) {
        counting_iterator temp = *this;
        ++*this;
        return temp;
    }
    int& operator*() { return ignore; }

    struct black_hole {
        void operator=(size_t) {}
    };
};
}  // namespace utl
