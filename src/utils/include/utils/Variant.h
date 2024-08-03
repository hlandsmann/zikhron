#include <typeinfo>
#include <variant> // IWYU pragma: export

namespace utl {

/******************************************************************************/
template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

/* variant_cast -------------------------------------------------------------- */
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
