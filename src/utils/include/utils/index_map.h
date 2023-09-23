#pragma once

#include <functional>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <type_traits>
#include <vector>

namespace utl {

template<typename T>
concept is_mutable = std::is_same_v<std::remove_const_t<T>, T>;

template<class T>
class index_map_iterator
{
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;
    using non_cv_T = std::remove_cv_t<T>;
    using vector_T = std::conditional_t<std::is_same_v<T, typename std::remove_const_t<T>>,
                                      std::vector<non_cv_T>,
                                      const std::vector<non_cv_T>>;

    index_map_iterator(vector_T& data, std::size_t index);

    auto operator*() const -> reference;
    auto operator++() -> index_map_iterator&;
    auto operator++(int) -> index_map_iterator;
    auto operator--() -> index_map_iterator&;
    auto operator--(int) -> index_map_iterator;
    auto operator==(const index_map_iterator& it) const -> bool;
    auto operator!=(const index_map_iterator& it) const -> bool;

private:
    // using non_const_T = T;
    std::reference_wrapper<vector_T> refData;
    std::size_t index;
};

template<class T>
class index_map
{
public:
    using key_type = unsigned;
    using mapped_type = T;
    using value_type = std::pair<key_type, mapped_type>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = mapped_type&;
    using const_reference = const mapped_type&;
    using pointer = mapped_type*;
    using const_pointer = const mapped_type*;
    using iterator = index_map_iterator<T>;
    using const_iterator = index_map_iterator<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using non_const_T = std::remove_const_t<T>;

    index_map() = default;
    [[nodiscard]] auto begin() -> iterator;
    [[nodiscard]] auto end()  -> iterator;
    [[nodiscard]] auto begin() const -> const_iterator;
    [[nodiscard]] auto end() const -> const_iterator;
    [[nodiscard]] auto cbegin() const -> const_iterator;
    [[nodiscard]] auto cend() const -> const_iterator;

    [[nodiscard]] auto size() const -> std::size_t;
    [[nodiscard]] auto empty() const -> bool;
    [[nodiscard]] auto at_id(unsigned id) const -> std::pair<std::size_t /* index */, T&>;
    [[nodiscard]] auto optional_index(unsigned id) const -> std::optional<std::size_t>;
    [[nodiscard]] auto contains(unsigned id) const -> bool;
    [[nodiscard]] auto operator[](std::size_t index) const -> const non_const_T&;
    [[nodiscard]] auto operator[](std::size_t index) -> T&
        requires is_mutable<T>;

    void push_back(std::pair<unsigned, const T&> id_value);
    void push_back(std::pair<unsigned, T&&> id_value);

    template<class... Args>
    auto emplace(unsigned id, Args&&... args) -> std::pair<std::size_t, std::reference_wrapper<T>>;

private:
    std::map<key_type, std::size_t> id_index;
    std::vector<T> data;
};

template<class T>
auto index_map<T>::begin() -> iterator
{
    return {data, 0};
}

template<class T>
auto index_map<T>::end() -> iterator
{
    return {data, data.size()};
}

template<class T>
auto index_map<T>::begin() const -> const_iterator
{
    return {data, 0};
}

template<class T>
auto index_map<T>::end() const -> const_iterator
{
    return {data, data.size()};
}

template<class T>
auto index_map<T>::cbegin() const -> const_iterator
{
    return {data, 0};
}

template<class T>
auto index_map<T>::cend() const -> const_iterator
{
    return {data, data.size()};
}

template<class T>
auto index_map<T>::size() const -> std::size_t
{
    return data.size();
}

template<class T>
auto index_map<T>::empty() const -> bool
{
    return data.empty();
}

template<class T>
auto index_map<T>::at_id(unsigned id) const -> std::pair<std::size_t, T&>
{
    std::size_t index = id_index.at(id);
    return {index, data[index]};
}

template<class T>
auto index_map<T>::optional_index(unsigned id) const -> std::optional<std::size_t>
{
    const auto it = id_index.find(id);
    if (it == id_index.end()) {
        return {};
    }
    return {it->second};
}

template<class T>
auto index_map<T>::contains(unsigned id) const -> bool
{
    return id_index.contains(id);
}

template<class T>
auto index_map<T>::operator[](std::size_t index) const -> const non_const_T&
{
    return data[index];
}

template<class T>
auto index_map<T>::operator[](std::size_t index) -> T&
    requires is_mutable<T>
{
    return data[index];
}

template<class T>
void index_map<T>::push_back(std::pair<unsigned, const T&> id_value)
{
    const auto& [id, value] = id_value;
    if (not id_index.contains(id)) {
        data.push_back(value);
        id_index[id] = data.size();
    } else {
        data[id_index.at(id)] = value;
    }
}

template<class T>
void index_map<T>::push_back(std::pair<unsigned, T&&> id_value)
{
    auto&& [id, value] = id_value;
    if (not id_index.contains(id)) {
        data.push_back(std::move(value));
        id_index[id] = data.size();
    } else {
        data[id_index.at(id)] = std::move(value);
    }
}

template<class T>
template<class... Args>
auto index_map<T>::emplace(unsigned id, Args&&... args) -> std::pair<std::size_t, std::reference_wrapper<T>>
{
    if (not id_index.contains(id)) {
        std::size_t index = data.size();
        reference ref = data.emplace_back(std::forward<Args>(args)...);
        id_index[id] = index;
        return {index, ref};
    }
    std::size_t index = id_index.at(id);
    auto it = std::next(data.begin(), index);
    return {index, *data.emplace(it, std::forward<Args>(args)...)};
}

template<class T>
index_map_iterator<T>::index_map_iterator(vector_T& data, std::size_t _index)
    : refData{data}, index{_index}
{}

template<class T>
auto index_map_iterator<T>::operator*() const -> reference
{
    auto& data = refData.get();
    return data[index];
}

template<class T>
auto index_map_iterator<T>::operator++() -> index_map_iterator&
{
    index++;
    return *this;
}

template<class T>
auto index_map_iterator<T>::operator++(int) -> index_map_iterator
{
    auto self = *this;
    index++;
    return self;
}

template<class T>
auto index_map_iterator<T>::operator--() -> index_map_iterator&
{
    index--;
    return *this;
}

template<class T>
auto index_map_iterator<T>::operator--(int) -> index_map_iterator
{
    auto self = *this;
    index--;
    return self;
}

template<class T>
auto index_map_iterator<T>::operator==(const index_map_iterator& it) const -> bool
{
    return (std::addressof(refData.get()) == std::addressof(it.refData.get()) && index == it.index);
}

template<class T>
auto index_map_iterator<T>::operator!=(const index_map_iterator& it) const -> bool
{
    return not(*this == it);
}

} // namespace utl
