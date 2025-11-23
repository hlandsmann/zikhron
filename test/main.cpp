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
    spdlog::sink_ptr sink;
    auto ring_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(128);
    ring_sink->set_pattern("[%H:%M:%S %e] [%L] %v");
    spdlog::logger tmplog("", ring_sink);
    tmplog.info("Hello world, {}", 42);
    tmplog.info("yes, {}", 42);

    for (const auto& str : ring_sink->last_formatted()) {
        fmt::print("{}", str);
    }
    for (const auto& str : ring_sink->last_formatted()) {
        fmt::print("{}", str);
    }

    return 0;

    auto injectorJpn = boost::di::make_injector(
            boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
            boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerJpn>(),
            boost::di::bind<dictionary::Dictionary>.to<dictionary::DictionaryJpn>(),
            boost::di::bind<Language>.to(Language::japanese));

    auto dictionary = injectorJpn.create<std::shared_ptr<dictionary::Dictionary>>();
    auto dictionaryJpn = std::dynamic_pointer_cast<const dictionary::DictionaryJpn>(dictionary);
    auto db = injectorJpn.create<std::shared_ptr<sr::DataBase>>();
    //
    // // auto list1 = {"人名", "名詞", "固有名詞", "人名", "一般"};
    // // 人名 -
    // //     g: person's name
    // // 名詞 -
    // //     g: noun
    // // 固有名詞 -
    // //     g: proper noun
    // // 人名 -
    // //     g: person's name
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"他", "感動詞", "フィラー"};
    // // 他 -
    // //     g: other (esp. people and abstract matters)
    // // 感動詞 -
    // //     かんどうし
    // //     g: interjection
    // // フィラー -(r)
    // //     g: filler
    //
    // // auto list = {"他", "感動詞", "一般"};
    // // 他 -
    // //     g: other (esp. people and abstract matters)
    // // 感動詞 -
    // //     g: interjection
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"他", "接続詞"};
    // // 他 -
    // //     g: other (esp. people and abstract matters)
    // // 接続詞 -
    // //     g: conjunction
    //
    // // auto list = {"体", "代名詞"};
    // // 体 -
    // //     g: body
    // // 代名詞 -
    // //     g: pattern
    // //     g: pronoun
    // //     g: synonym
    // //     pos: noun
    //
    // // auto list = {"体", "名詞", "助動詞語幹"};
    // // 体 -
    // //     g: body
    // // 名詞 -
    // //     g: noun
    // // 助動詞語幹
    // //     auxilary verb (word?) stem
    //
    // // auto list = {"体", "名詞", "普通名詞", "サ変可能"};
    // // 体 -
    // //     g: body
    // // 名詞 -
    // //     g: noun
    // // 普通名詞 -
    // //     g: common noun
    // // サ変可能 -(r)
    // //     can be changed
    //
    // // auto list = {  "体","名詞","普通名詞","サ変形状詞可能"};
    // // 体 -
    // //     g: body
    // // 名詞 -
    // //     g: noun
    // // 普通名詞 -
    // //     g: common noun
    // // サ変形状詞可能 -(r)
    // //     sa inflectional verb possible
    //
    // // auto list = {"体", "名詞", "普通名詞", "一般"};
    // // 体 -
    // //     g: body
    // // 名詞 -
    // //     g: noun
    // // 普通名詞 -
    // //     g: common noun
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"体", "名詞", "普通名詞", "副詞可能"};
    // // 体 -
    // //     g: body
    // // 名詞 -
    // //     g: noun
    // // 普通名詞 -
    // //     g: common noun
    // // 副詞可能 -(r)
    // //     adverb possible
    //
    // // auto list = {"体", "名詞", "普通名詞", "助数詞可能"};
    // // 体 -
    // //     g: body
    // // 名詞 -
    // //     g: noun
    // // 普通名詞 -
    // //     g: common noun
    // // 助数詞可能 -(r)
    // //     particle possible
    //
    // // auto list = {  "体","名詞","普通名詞","形状詞可能"};
    // // 体 -
    // //     g: body
    // // 名詞 -
    // //     g: noun
    // // 普通名詞 -
    // //     g: common noun
    // // 形状詞可能 -(r)
    // //     shape word possible
    //
    // // auto list = {"係助", "助詞", "係助詞"};
    // // 係助 -
    // //     i: used in dictionaries
    // //     g: binding particle
    // // 助詞 -
    // //     g: particle
    // //     g: postpositional word in Japanese grammar that functions as an auxiliary to a main word
    // // 係助詞 -
    // //     g: binding particle (e.g. "ha", "mo", "koso", "shika")
    // //     g: connecting particle
    // //     g: linking particle
    //
    // // auto list = {"副助", "助詞", "副助詞"};
    // // 副助 -(r)
    // //     assisstant
    // // 助詞 -
    // //     g: particle
    // //     g: postpositional word in Japanese grammar that functions as an auxiliary to a main word
    // // 副助詞 -
    // //     g: adverbial particle (e.g. "bakari", "nado", "kurai", "hodo")
    //
    // // auto list = {"助動", "助動詞"};
    // // 助動 -
    // //     i: part of speech tag used in dictionaries
    // //     g: inflecting dependent word
    // // 助動詞 -
    // //     g: auxiliary verb (in languages other than Japanese)
    // //     g: bound auxiliary
    // //     g: inflecting dependent word (in Japanese)
    // //     pos: noun
    //
    // // auto list = {"助動", "形状詞", "助動詞語幹"};
    // // 助動 -
    // //     i: part of speech tag used in dictionaries
    // //     g: inflecting dependent word
    // // 形状詞 -
    // //     g: adjectival noun
    // //     g: na-adjective
    //
    // // auto list = {"助数", "接尾辞", "名詞的", "助数詞"};
    // // 助数 -(r)
    // //      fraction
    // // 接尾辞 -
    // //     g: suffix
    // // 名詞的 -(r)
    // // 助数詞 -
    // //     g: counter
    // //     g: counter word
    // //     g: measure word
    //
    // // auto list = {"名", "名詞", "固有名詞", "人名", "名"};
    // // 名 -
    // //     g: name
    // // 名詞 -
    // //     g: noun
    // // 固有名詞 -
    // //     g: proper noun
    // // 人名 -
    // //     g: person's name
    // // 名 -
    // //     g: name
    //
    // // auto list = {"固有名", "名詞", "固有名詞", "一般"};
    // // 固有名 -(r)
    // //      proper name
    // // 名詞 -
    // //     g: noun
    // // 固有名詞 -
    // //     g: proper noun
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"国", "名詞", "固有名詞", "地名", "国"};
    // // 国 -
    // //     g: country
    // // 名詞 -
    // //     g: noun
    // // 固有名詞 -
    // //     g: proper noun
    // // 地名 -
    // //     g: place name
    // //     g: toponym
    // // 国 -
    // //     g: country
    //
    // // auto list = {"地名", "名詞", "固有名詞", "地名", "一般"};
    // // 地名 -
    // //     g: toponym
    // // 名詞 -
    // //     g: noun
    // // 固有名詞 -
    // //     g: proper noun
    // // 地名 -
    // //     g: toponym
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"姓", "名詞", "固有名詞", "人名", "姓"};
    // // 姓 -
    // //     g: family name
    // // 名詞 -
    // //     g: noun
    // // 固有名詞 -
    // //     g: proper noun
    // // 人名 -
    // //     g: person's name
    // // 姓 -
    // //     g: family name
    //
    // // auto list = {"接助", "助詞", "接続助詞"};
    // // 接助 -
    // //     g: conjunctive particle
    // // 助詞 -
    // //     g: particle
    // // 接続助詞 -
    // //     g: conjunction particle (e.g. "ba", "kara", "keredo", "nagara")
    //
    // // auto list = {"接尾体", "接尾辞", "名詞的", "サ変可能"};
    // // 接尾体 -(r)
    // // 接尾辞 -
    // //     g: suffix
    // // 名詞的 -(r)
    // // サ変可能 -(r)
    //
    // // auto list = {"接尾体", "接尾辞", "名詞的", "一般"};
    // // 接尾体 -(r)
    // // 接尾辞 -
    // //     g: suffix
    // // 名詞的 -(r)
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"接尾体", "接尾辞", "名詞的", "副詞可能"};
    // // 接尾体 -(r)
    // // 接尾辞 -
    // //     g: suffix
    // // 名詞的 -(r)
    // // 副詞可能 -(r)
    //
    // // auto list = {"接尾用", "接尾辞", "動詞的"};
    // // 接尾用 -(r)
    // // 接尾辞 -
    // //     g: suffix
    // // 動詞的 -(r)
    //
    // // auto list = {"接尾相", "接尾辞", "形容詞的"};
    // // 接尾相 -(r)
    // // 接尾辞 -
    // //     g: suffix
    // // 形容詞的 -
    // //     g: adjectival
    //
    // // auto list = {"接尾相", "接尾辞", "形状詞的"};
    // // 接尾相 -(r)
    // // 接尾辞 -
    // //     g: suffix
    // // 形状詞的 -(r)
    //
    // // auto list = {"接頭", "接頭辞"};
    // // 接頭 -
    // //     g: prefix
    // // 接頭辞 -
    // //     g: prefix
    //
    // // auto list = {"数", "名詞", "数詞"};
    // // 数 -
    // //     g: frequently
    // //     g: number
    // // 名詞 -
    // //     g: noun
    // // 数詞 -
    // //     g: numeral
    //
    // // auto list = {"格助", "助詞", "格助詞"};
    // // 格助 -
    // //     g: case-marking particle
    // // 助詞 -
    // //     g: particle
    // //     g: postpositional word in Japanese grammar that functions as an auxiliary to a main word
    // // 格助詞 -
    // //     g: case-marking particle (e.g. "ga", "no", "wo", "ni")
    //
    // // auto list = {"準助", "助詞", "準体助詞"};
    // // 準助 -(r)
    // // 助詞 -
    // //     g: particle
    // //     g: postpositional word in Japanese grammar that functions as an auxiliary to a main word
    // // 準体助詞 -
    // //     i: e.g. the の in 行くのをやめる
    // //     g: particle that attaches to a phrase and acts on the whole phrase
    //
    // // auto list = {"用", "動詞", "一般"};
    // // 用 -
    // //     g: purpose
    // //     g: task
    // //     g: use
    // // 動詞 -
    // //     g: verb
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"用", "動詞", "非自立可能"};
    // // 用 -
    // //     g: use
    // // 動詞 -
    // //     g: verb
    // // 非自立可能 -(r)
    //
    // // auto list = {"相", "副詞"};
    // // 相 -
    // //     g: convention
    // //     g: appearance
    // //     g: aspect
    // // 副詞 -
    // //     g: adverb
    //
    // // auto list = {"相", "形容詞", "一般"};
    // // 相 -
    // //     g: convention
    // //     g: appearance
    // //     g: aspect
    // // 形容詞 -
    // //     g: adjective
    // //     g: i-adjective (in Japanese)
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"相", "形容詞", "非自立可能"};
    // // 相 -
    // //     g: convention
    // //     g: appearance
    // //     g: aspect
    // // 形容詞 -
    // //     g: adjective
    // //     g: i-adjective (in Japanese)
    // // 非自立可能 -(r)
    //
    // // auto list = {  "相","形状詞","タリ"};
    // // 相 -
    // //     g: convention
    // //     g: appearance
    // //     g: aspect
    // // 形状詞 -
    // //     g: adjectival noun
    // //     g: na-adjective
    // // タリ -(r)
    //
    // // auto list = {"相", "形状詞", "一般"};
    // // 相 -
    // //     g: convention
    // //     g: appearance
    // //     g: aspect
    // // 形状詞 -
    // //     g: adjectival noun
    // //     g: na-adjective
    // // 一般 -
    // //     g: common
    //
    // // auto list = {  "相","連体詞"};
    // // 相 -
    // //     g: convention
    // //     g: appearance
    // //     g: aspect
    // // 連体詞 -
    // //     g: adnominal adjective
    // //     g: pre-noun adjectival
    //
    // // auto list = {"終助", "助詞", "終助詞"};
    // // 終助 -
    // //     i: part of speech tag in dictionaries
    // //     g: sentence-ending particle (e.g. "ka", "na", "yo", "kashira")
    // // 助詞 -
    // //     g: particle
    // //     g: postpositional word in Japanese grammar that functions as an auxiliary to a main word
    // // 終助詞 -
    // //     g: sentence-ending particle (e.g. "ka", "na", "yo", "kashira")
    //
    // // auto list = {"補助", "空白"};
    // // 補助 -
    // //     g: supplement
    // //     g: support
    // // 空白 -
    // //     g: blank
    // //     g: blank space (in a document)
    // //     g: gap
    // //     g: whitespace
    //
    // // auto list = {  "補助","補助記号","一般"};
    // //     g: supplement
    // //     g: support
    // // 補助記号 -
    // //     g: supplementary symbol (number, punctuation, etc.)
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"補助", "補助記号", "句点"};
    // // 補助 -
    // //     g: supplement
    // //     g: support
    // // 補助記号 -
    // //     g: supplementary symbol (number, punctuation, etc.)
    // // 句点 -
    // //     g: full stop
    // //     g: period
    //
    // // auto list = {  "補助","補助記号","括弧閉"};
    // // 補助 -
    // //     g: supplement
    // //     g: support
    // // 補助記号 -
    // //     g: supplementary symbol (number, punctuation, etc.)
    //
    // // auto list = {"補助", "補助記号", "括弧開"};
    // // 補助 -
    // //     g: supplement
    // //     g: support
    // // 補助記号 -
    // //     g: supplementary symbol (number, punctuation, etc.)
    // // 括弧開 -(r)
    //
    // // auto list = {"補助", "補助記号", "読点"};
    // // 補助 -
    // //     g: supplement
    // //     g: support
    // // 補助記号 -
    // //     g: supplementary symbol (number, punctuation, etc.)
    // // 読点 -
    // //     g: comma
    //
    // // auto list = {"補助", "補助記号", "ＡＡ", "一般"};
    // // 補助 -
    // //     g: supplement
    // //     g: support
    // // 補助記号 -
    // //     g: supplementary symbol (number, punctuation, etc.)
    // // ＡＡ -(r)
    // // 一般 -
    // //     g: common
    //
    // // auto list = {"補助", "補助記号", "ＡＡ", "顔文字"};
    // // 補助 -
    // //     g: supplement
    // //     g: support
    // // 補助記号 -
    // //     g: supplementary symbol (number, punctuation, etc.)
    // // ＡＡ -(r)
    // // 顔文字 -
    // //     g: emoticon
    // //     g: kaomoji
    //
    // // auto list = {"記号", "記号", "一般"};
    // // 記号 -
    // //     g: mark
    // //     g: sign
    // //     g: symbol
    // // 記号 -
    // //     g: mark
    // //     g: sign
    // //     g: symbol
    // // 一般 -
    // //     g: common
    //
    // auto list = {"記号", "記号", "文字"};
    // // 記号 -
    // //     g: mark
    // //     g: sign
    // //     g: symbol
    // // 記号 -
    // //     g: mark
    // //     g: sign
    // //     g: symbol
    // // 文字 -
    // //     g: character
    // //     g: letter (of an alphabet)
    // //     g: writing
    //

    auto list = {"別"};
    for (const std::string item : list) {
        auto entry = dictionaryJpn->getEntryByKanji(item);
        if (!entry.definition.empty()) {
            fmt::print("{} -\n", fmt::join(entry.kanji, ", "));
            for (const auto& def : entry.definition) {
                fmt::print("    {}\n", fmt::join(def.reading, ", "));
                fmt::print("    i: {}\n", fmt::join(def.info, "\n    i: "));
                fmt::print("    g: {}\n", fmt::join(def.glossary, "\n    g: "));
                for (const auto& pos : def.pos) {
                    fmt::print("    pos: {}\n", pos);
                }
                // fmt::print("    p: {}\n", fmt::join(def.pos, "\n    p: "));
            }
        } else {
            entry = dictionaryJpn->getEntryByReading(item);
            fmt::print("{} -(r)\n", item);
            for (const auto& def : entry.definition) {
                fmt::print("    {}\n", fmt::join(def.reading, ", "));
                fmt::print("    i: {}\n", fmt::join(def.info, "\n    i: "));
                fmt::print("    g: {}\n", fmt::join(def.glossary, "\n    g: "));
                // fmt::print("    pos: {}\n", fmt::join(def.pos, "\n    pos: "));
            }
        }
    }
    //
    // // auto treeWalker = sr::ITreeWalker::createTreeWalker(std::move(db));
    // // auto& cardMeta = treeWalker->getNextCardChoice();
    // // if (cardMeta.getCardId() == 0) {
    // //     spdlog::info("No card found!");
    // //     return 0;
    // // }
    // // auto cardId = cardMeta.getCardId();
    // // spdlog::info("show cardId: {}, size: {}", cardId, cardMeta.getRelevantEase().size());
    //
    // return 0;
    std::cout << "cout" << std::endl;
    std::cerr << "cerr";
}
