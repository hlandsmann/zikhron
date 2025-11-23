#include <annotation/AdaptJiebaDict.h>
#include <annotation/FreqDictionary.h>
#include <annotation/JieBa.h>
#include <annotation/Tokenizer.h>
#include <annotation/TokenizerChi.h>
#include <annotation/TokenizerJpn.h>
#include <database/CardPack.h>
#include <database/CardPackDB.h>
#include <database/SpacedRepetitionData.h>
#include <database/TokenizationChoiceDB.h>
#include <database/TokenizationChoiceDbChi.h>
#include <database/WordDB.h>
#include <database/WordDB_jpn.h>
#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
#include <dictionary/DictionaryJpn.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <spaced_repetition/DataBase.h>
#include <spaced_repetition/ITreeWalker.h>
#include <spaced_repetition/Scheduler.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>
#include <utils/format.h>

#include <algorithm>
#include <boost/di.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <ratio>
#include <set>
#include <span>
#include <string>
#include <utility>

namespace fs = std::filesystem;

auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg_json = path_to_exe.parent_path() / "config.json";
    return std::make_shared<zikhron::Config>(path_to_exe.parent_path());
}

auto main() -> int
{
    std::set<utl::CharU8> kanjis;
    auto injectorJpn = boost::di::make_injector(
            boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
            boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerJpn>(),
            boost::di::bind<dictionary::Dictionary>.to<dictionary::DictionaryJpn>(),
            boost::di::bind<database::WordDB>.to<database::WordDB_jpn>(),
            boost::di::bind<Language>.to(Language::japanese));

    auto db = injectorJpn.create<std::shared_ptr<sr::DataBase>>();
    auto dictionary = injectorJpn.create<std::shared_ptr<dictionary::Dictionary>>();
    auto dictionaryJpn = std::dynamic_pointer_cast<const dictionary::DictionaryJpn>(dictionary);
    spdlog::info("Hello world");

    for (const auto& [keb, _] : dictionaryJpn->kanjiToIndex) {
        const auto& kebU8 = utl::StringU8{keb};
        for (const auto& charU8 : kebU8.getChars()) {
            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv_utf8_utf32;
            std::uint32_t val{};
            for (const auto& x : conv_utf8_utf32.from_bytes(charU8.string())) {
                // fmt::print("{:x} ", static_cast<std::uint32_t>(x));
                val = static_cast<std::uint32_t>(x);
            }
            if (val < 0x4e00) {
                continue;
            }
            if (val > 0x9f9d) {
                continue;
            }
            kanjis.insert(charU8);
        }
    }

    fmt::print("\"\"\n\"");
    int count = 0;
    kanjis.insert(utl::CharU8{"♪"});
    kanjis.insert(utl::CharU8{"⸺"});
    // 4e00 9f9d
    for (const auto& kanji : kanjis) {
        fmt::print("{}", kanji);
        if (++count % 45 == 0) {
            fmt::print("\"\n\"");
        }
    }
    fmt::print("\";\n");

    spdlog::info("Kanjis: {}", kanjis.size());

    return 0;
}
