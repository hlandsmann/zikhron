#pragma once

#include <memory>

namespace jumanpp {
class Jumanpp;
}

namespace annotation {

class JpnTokenizer
{
public:
    JpnTokenizer();
    void tokenize(const std::string& text);

private:
    std::shared_ptr<jumanpp::Jumanpp> jumanppPtr;
};

} // namespace annotation
