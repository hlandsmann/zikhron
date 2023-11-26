#include <typeinfo>
#include <variant>
namespace utl {
template<class... Args>
auto variant_cast(auto* derived) -> std::variant<Args*...>
{
    std::variant<Args*...> result;
    bool found = (([&] {
                      auto* v = dynamic_cast<Args*>(derived);
                      if (v != nullptr) {
                          result = v;
                          return true;
                      }
                      return false;
                  }())
                  || ...);
    if (!found) {
        throw std::bad_cast();
    }
    return result;
}
} // namespace utl
