#include "MecabWrapper.h"

#include "Mecab.h"

#include <mecab/mecab.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>
#include <utils/string_split.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace annotation {
MecabWrapper::MecabWrapper()
    : tagger{std::shared_ptr<MeCab::Tagger>(MeCab::createTagger("--dicdir=/home/harmen/zikhron/dictionary/unidic-novel"))}
    , log{std::make_unique<spdlog::logger>("", std::make_shared<spdlog::sinks::null_sink_mt>())}
{}

auto MecabWrapper::split(const std::string& text) const -> std::vector<MecabToken>
{
    log->warn("Text:  {}", text);
    std::vector<MecabToken> result;
    std::string mecabOut = tagger->parse(text.c_str());
    log->warn("Text out: \n {}", mecabOut);
    auto rest = std::string_view{mecabOut};
    while (true) {
        auto tagged = utl::split_front(rest, '\n');
        if (tagged.empty() || tagged == "EOS") {
            break;
        }
        auto surface = utl::split_front(tagged, '\t');
        auto pos1 = utl::split_front(tagged, ',');
        auto pos2 = utl::split_front(tagged, ',');
        auto pos3 = utl::split_front(tagged, ',');
        auto pos4 = utl::split_front(tagged, ',');
        auto conjugationType = utl::split_front(tagged, ',');
        auto conjugationShape = utl::split_front(tagged, ',');
        auto lemmaReading = utl::split_front(tagged, ',');
        auto lemma = utl::split_front(tagged, ',');
        auto orth = utl::split_front(tagged, ',');
        auto pronounciation = utl::split_front(tagged, ',');
        auto orthBase = utl::split_front(tagged, ',');
        auto pronounciationBase = utl::split_front(tagged, ',');
        auto goshu = utl::split_front(tagged, ',');
        auto iType = utl::split_front(tagged, ',');
        auto iForm = utl::split_front(tagged, ',');
        auto fType = utl::split_front(tagged, ',');
        auto fForm = utl::split_front(tagged, ',');
        auto iConType = utl::split_front(tagged, ',');
        auto fConType = utl::split_front(tagged, ',');
        auto lemmaType = utl::split_front(tagged, ',');

        surface = (surface == "*") ? "" : surface;
        pos1 = (pos1 == "*") ? "" : pos1;
        pos2 = (pos2 == "*") ? "" : pos2;
        pos3 = (pos3 == "*") ? "" : pos3;
        pos4 = (pos4 == "*") ? "" : pos4;
        conjugationType = (conjugationType == "*") ? "" : conjugationType;
        conjugationShape = (conjugationShape == "*") ? "" : conjugationShape;
        lemmaReading = (lemmaReading == "*") ? "" : lemmaReading;
        lemma = (lemma == "*") ? "" : lemma;
        orth = (orth == "*") ? "" : orth;
        pronounciation = (pronounciation == "*") ? "" : pronounciation;
        orthBase = (orthBase == "*") ? "" : orthBase;
        pronounciationBase = (pronounciationBase == "*") ? "" : pronounciationBase;
        goshu = (goshu == "*") ? "" : goshu;
        iType = (iType == "*") ? "" : iType;
        iForm = (iForm == "*") ? "" : iForm;
        fType = (fType == "*") ? "" : fType;
        fForm = (fForm == "*") ? "" : fForm;
        iConType = (iConType == "*") ? "" : iConType;
        fConType = (fConType == "*") ? "" : fConType;
        lemmaType = (lemmaType == "*") ? "" : lemmaType;
        auto mecabToken = MecabToken{
                .surface = std::string{surface},
                .pos1 = std::string{pos1},
                .pos2 = std::string{pos2},
                .pos3 = std::string{pos3},
                .pos4 = std::string{pos4},
                .conjugationType = std::string{conjugationType},
                .conjugationShape = std::string{conjugationShape},
                .lemmaReading = std::string{lemmaReading},
                .lemma = std::string{lemma},
                .orth = std::string{orth},
                .pronounciation = std::string{pronounciation},
                .orthBase = std::string{orthBase},
                .pronounciationBase = std::string{pronounciationBase},
                .goshu = std::string{goshu},
                .iType = std::string{iType},
                .iForm = std::string{iForm},
                .fType = std::string{fType},
                .fForm = std::string{fForm},
                .iConType = std::string{iConType},
                .fConType = std::string{fConType},
                .lemmaType = std::string{lemmaType},
        };
        result.push_back(mecabToken);

        // spdlog::info("{} - {}, {}", surface, lemma, lemmaType);
        // MecabToken token = {.surface = utl::split_front(tagged, '\t'),
        //                     .lemma = ""};
    }

    return result;
}

void MecabWrapper::setDebugSink(spdlog::sink_ptr sink)
{
    log = std::make_unique<spdlog::logger>("", sink);
}
} // namespace annotation
