#pragma once
#include <memory>
#include <string>
#include <vector>

namespace jumanpp {
class Jumanpp;
}

namespace annotation {

struct JumanppToken
{
    std::string surface;
    std::string pos;
    std::string subpos;
    std::string conjType;
    std::string conjForm;
    std::string baseform;
    std::string reading;
    std::string canonicForm;
};

class JumanppWrapper

{
public:
    JumanppWrapper();
    [[nodiscard]] auto tokenize(const std::string& text) const -> std::vector<JumanppToken>;

private:
    std::shared_ptr<jumanpp::Jumanpp> jumanppPtr;
};
} // namespace annotation
