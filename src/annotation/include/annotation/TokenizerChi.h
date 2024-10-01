#pragma once
#include "FreqDictionary.h"
#include "JieBa.h"
#include "Rules.h"
#include "Token.h"
#include "Tokenizer.h"
#include "misc/TokenizationChoice.h"

#include <database/WordDB.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <memory>
#include <set>
#include <span>
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
    std::vector<std::vector<utl::StringU8>> candidates;
};

class TokenizerChi : public Tokenizer
{
public:
    TokenizerChi(std::shared_ptr<zikhron::Config> config, std::shared_ptr<database::WordDB> wordDB);
    ~TokenizerChi() override = default;
    TokenizerChi(const TokenizerChi&) = default;
    TokenizerChi(TokenizerChi&&) = default;
    auto operator=(const TokenizerChi&) -> TokenizerChi& = default;
    auto operator=(TokenizerChi&&) -> TokenizerChi& = default;

    [[nodiscard]] auto split(const std::string& text) const -> std::vector<Token> override;
    [[nodiscard]] auto getAlternatives(const std::string& text, const std::vector<Token>& currentSplit) const -> std::vector<Alternative>;
    auto getSplitForChoices(const TokenizationChoiceVec& choices,
                            const std::string& text,
                            const std::vector<Token>& currentSplit)
            -> std::vector<Token>;

private:
    struct CandidateSplit
    {
        utl::StringU8 key;
        std::vector<std::vector<AToken>> candidates;

        [[nodiscard]] auto empty() const -> bool { return candidates.empty(); }
    };

    [[nodiscard]] auto getCandidates(const utl::StringU8& text,
                                     const dictionary::DictionaryChi& dict) const -> std::vector<std::vector<AToken>>;
    [[nodiscard]] static auto previousIndex(const std::vector<std::size_t>& currentVec,
                                            std::size_t currentIndex,
                                            std::span<const std::vector<AToken>> tokens) -> std::size_t;
    [[nodiscard]] static auto lastIndex(std::vector<std::size_t>& cvec,
                                        std::span<const std::vector<AToken>> tokens) -> std::size_t;
    [[nodiscard]] static auto doPseudoPerm(std::vector<std::size_t>& cvec,
                                           std::span<const std::vector<AToken>> tokens) -> bool;
    [[nodiscard]] static auto verifyPerm(std::vector<std::size_t>& perm,
                                         std::span<const std::vector<AToken>> tokens) -> bool;
    [[nodiscard]] static auto genTokenVector(const std::vector<std::size_t>& vec,
                                             std::span<const std::vector<AToken>> tokens) -> std::vector<AToken>;
    [[nodiscard]] static auto getAlternativeATokenVector(std::span<const std::vector<AToken>> tokens)
            -> std::vector<std::vector<AToken>>;
    [[nodiscard]] static auto chooseCombination(std::span<const std::vector<AToken>> tokens)
            -> std::vector<AToken>;
    [[nodiscard]] static auto splitCandidates(std::span<std::vector<AToken>> candidates) -> CandidateSplit;

    [[nodiscard]] static auto findEndItForLengthOfAlternativeSplit(std::vector<Token>::const_iterator firstSplit,
                                                                   const CandidateSplit& candidateSplit)
            -> std::vector<Token>::const_iterator;
    [[nodiscard]] auto joinMissed(const std::vector<Token>& splitVector, const std::string& text) const
            -> std::vector<Token>;
    [[nodiscard]] auto splitFurther(const std::string& text) const -> std::vector<AToken>;

    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<database::WordDB> wordDB;
    JieBa jieba;
    Rules rules;
    std::shared_ptr<FreqDictionary> freqDictionary;
};

} // namespace annotation
