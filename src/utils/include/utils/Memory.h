#pragma once
#include <memory>
#include <utility>

namespace utl {

template<class T>
auto make_shared(auto&&... args) -> std::shared_ptr<T>
{
    return std::make_shared<T>(std::forward<decltype(args)...>(args...));
}

} // namespace utl
