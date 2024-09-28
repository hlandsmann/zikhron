#include "detail/JumanppWrapper.h"

#include "Jumanpp.h"

#include <memory>
#include <string>
#include <vector>

namespace annotation {
JumanppWrapper::JumanppWrapper()
    : jumanppPtr{std::make_shared<jumanpp::Jumanpp>()}
{}

auto JumanppWrapper::tokenize(const std::string& text) const -> std::vector<JumanppToken>
{
    return jumanppPtr->tokenize(text);
}
} // namespace annotation
