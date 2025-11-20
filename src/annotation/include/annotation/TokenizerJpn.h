#pragma once
#include "Mecab.h"
#include "Sudachi.h"
#include "Token.h"
#include "Tokenizer.h"

#include <database/WordDB.h>
#include <database/WordDB_jpn.h>
#include <dictionary/DictionaryJpn.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <utils/ProcessPipe.h>

#include <memory>
#include <string>
#include <vector>

namespace annotation {

struct JpnToken
{
    std::string baseform;
    std::string surface;
    std::string reading;
};

class TokenizerJpn : public Tokenizer
{
public:
    TokenizerJpn(std::shared_ptr<database::WordDB> wordDB, std::unique_ptr<Sudachi> sudachi);
    [[nodiscard]] auto split(const std::string& text) const -> std::vector<Token> override;
    void setDebugSink(spdlog::sink_ptr sink) override;

private:
    [[nodiscard]] auto split_mecab(const std::string& text) const -> std::vector<Token>;
    [[nodiscard]] auto split_sudachi(const std::string& text) const -> std::vector<Token>;
    std::shared_ptr<Mecab> mecab;
    std::unique_ptr<Sudachi> sudachi;
    std::shared_ptr<database::WordDB_jpn> wordDB;
    std::shared_ptr<dictionary::DictionaryJpn> jpnDictionary;
    std::unique_ptr<spdlog::logger> log;
};

} // namespace annotation
