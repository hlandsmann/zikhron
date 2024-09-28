#pragma once
#include <dictionary/JpnDictionary.h>

#include <memory>
#include <string>
#include <vector>

namespace annotation {
class JumanppWrapper;

struct JpnToken
{
    std::string baseform;
    std::string surface;
    std::string reading;
};

class JpnTokenizer
{
public:
    JpnTokenizer(std::shared_ptr<dictionary::JpnDictionary> jpnDictionary);
    [[nodiscard]] auto tokenize(const std::string& text) const -> std::vector<JpnToken>;

private:
    std::shared_ptr<JumanppWrapper> jumanppWrapper;
    std::shared_ptr<dictionary::JpnDictionary> jpnDictionary;
};

} // namespace annotation
