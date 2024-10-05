#pragma once
#include "Mecab.h"

#include <memory>
#include <string>
#include <vector>

namespace MeCab {
class Tagger;
}

namespace annotation {

class MecabWrapper
{
public:
    MecabWrapper();
    [[nodiscard]] auto split(const std::string& text) const -> std::vector<MecabToken>;

private:
    std::shared_ptr<MeCab::Tagger> tagger;
};
} // namespace annotation
