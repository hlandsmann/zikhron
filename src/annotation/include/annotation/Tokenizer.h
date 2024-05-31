#pragma once
#include "FreqDictionary.h"
#include "JieBa.h"
#include "Rules.h"
#include "Token.h"
#include "WordDB.h"

#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace annotation {

struct AToken
{
    utl::StringU8 key;
    utl::StringU8 str;
};

struct Alternative
{
    std::vector<utl::StringU8> current;
    std::vector<std::vector<utl::StringU8>> alts;
};

class Tokenizer
{
public:
    Tokenizer(std::shared_ptr<zikhron::Config> config, std::shared_ptr<WordDB> wordDB);

    [[nodiscard]] auto split(const std::string& text) -> std::vector<Token>;
    auto getAlternatives(const std::string& text, const std::vector<Token>& currentSplit) -> std::vector<Alternative>;

private:
    auto joinMissed(const std::vector<Token>& splitVector, const std::string& text) -> std::vector<Token>;
    auto splitFurther(const std::string& text) -> std::vector<AToken>;
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<WordDB> wordDB;
    JieBa jieba;
    Rules rules;
    std::shared_ptr<FreqDictionary> freqDictionary;
    std::set<std::string> allWords;
};

} // namespace annotation
