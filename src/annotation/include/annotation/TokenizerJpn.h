#pragma once
#include "Mecab.h"
#include "Token.h"
#include "Tokenizer.h"

#include <database/WordDB.h>
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

class TokenizerJpn : public Tokenizer
{
public:
    TokenizerJpn(std::shared_ptr<database::WordDB> wordDB);
    [[nodiscard]] auto split(const std::string& text) const -> std::vector<Token> override;
    [[nodiscard]] auto debugString() const -> std::string override;

private:
    std::shared_ptr<Mecab> mecab;
    std::shared_ptr<database::WordDB> wordDB;
    std::shared_ptr<const dictionary::DictionaryJpn> jpnDictionary;

    mutable std::string lastDebugString;
};

} // namespace annotation
