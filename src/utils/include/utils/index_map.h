#pragma once

#include <functional>
#include <iterator>
#include <map>
#include <ranges>
#include <tuple>
#include <vector>

namespace utl {
template <class T> class index_map_iterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    index_map_iterator(std::vector<T>& data, std::size_t index);

    auto operator*() const -> reference;
    auto operator++() -> index_map_iterator&;
    auto operator++(int) -> index_map_iterator;
    auto operator--() -> index_map_iterator&;
    auto operator--(int) -> index_map_iterator;
    auto operator==(const index_map_iterator& it) const -> bool;
    auto operator!=(const index_map_iterator& it) const -> bool;

private:
    std::reference_wrapper<std::vector<T>> refData;
    std::size_t index;
};

template <class T> class index_map {
public:
    using key_type = unsigned;
    using mapped_type = T;
    using value_type = std::pair<key_type, mapped_type>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = index_map_iterator<T>;
    using const_iterator = index_map_iterator<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    index_map() = default;
    [[nodiscard]] auto begin() const -> iterator;
    [[nodiscard]] auto end() const -> iterator;
    [[nodiscard]] auto cbegin() const -> const_iterator;
    [[nodiscard]] auto cend() const -> const_iterator;

    [[nodiscard]] auto size() const -> std::size_t;
    [[nodiscard]] auto empty() const -> bool;
    [[nodiscard]] auto at_id(unsigned id) const -> std::pair<std::size_t /* index */, T&>;
    [[nodiscard]] auto operator[](std::size_t index) const -> T&;

private:
    std::map<unsigned, std::size_t> id_index;
    std::vector<T> data;
};

template <class T> auto index_map<T>::begin() const -> iterator {}
template <class T> auto index_map<T>::end() const -> iterator {}
template <class T> auto index_map<T>::cbegin() const -> const_iterator {}
template <class T> auto index_map<T>::cend() const -> const_iterator {}
template <class T> auto index_map<T>::size() const -> std::size_t { return data.size(); }

template <class T> auto index_map<T>::empty() const -> bool { return data.empty(); }

template <class T> auto index_map<T>::at_id(unsigned id) const -> std::pair<std::size_t, T&> {
    std::size_t index = id_index.at(id);
    return {index, data[index]};
}

template <class T> auto index_map<T>::operator[](std::size_t index) const -> T& { return data[index]; }

template <class T>
index_map_iterator<T>::index_map_iterator(std::vector<T>& data, std::size_t index)
    : refData{data}, index{index} {}

template <class T> auto index_map_iterator<T>::operator*() const -> reference {}

template <class T> auto index_map_iterator<T>::operator++() -> index_map_iterator& {}

template <class T> auto index_map_iterator<T>::operator++(int) -> index_map_iterator {}

template <class T> auto index_map_iterator<T>::operator--() -> index_map_iterator& {}

template <class T> auto index_map_iterator<T>::operator--(int) -> index_map_iterator {}

template <class T> auto index_map_iterator<T>::operator==(const index_map_iterator& it) const -> bool {}

template <class T> auto index_map_iterator<T>::operator!=(const index_map_iterator& it) const -> bool {}

}  // namespace utl
