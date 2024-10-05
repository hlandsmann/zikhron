#include "Mecab.h"

#include "MecabWrapper.h"

#include <memory>
#include <string>
#include <vector>

namespace annotation {

Mecab::Mecab()
    : mecabWrapper{std::make_shared<MecabWrapper>()}
{}

auto Mecab::split(const std::string& text) -> std::vector<MecabToken>
{
    return mecabWrapper->split(text);
}
} // namespace annotation
