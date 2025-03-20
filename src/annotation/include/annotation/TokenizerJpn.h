#pragma once
#include "Mecab.h"
#include "Token.h"
#include "Tokenizer.h"

#include <database/WordDB.h>
#include <dictionary/DictionaryJpn.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>

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
    void setDebugSink(spdlog::sink_ptr sink) override;

private:
    std::shared_ptr<Mecab> mecab;
    std::shared_ptr<database::WordDB> wordDB;
    std::shared_ptr< dictionary::DictionaryJpn> jpnDictionary;
    std::unique_ptr<spdlog::logger> log;
};

} // namespace annotation
