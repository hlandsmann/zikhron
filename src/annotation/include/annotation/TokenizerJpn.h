#pragma once
#include "Token.h"
#include "Tokenizer.h"

#include <dictionary/DictionaryJpn.h>

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

class JpnTokenizer //: public Tokenizer
{
public:
    JpnTokenizer(std::shared_ptr<dictionary::DictionaryJpn> jpnDictionary);
    [[nodiscard]] auto split(const std::string& text) const -> std::vector<Token> ;

private:
    std::shared_ptr<JumanppWrapper> jumanppWrapper;
    std::shared_ptr<dictionary::DictionaryJpn> jpnDictionary;
};

} // namespace annotation
