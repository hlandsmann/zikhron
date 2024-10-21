#include <annotation/AdaptJiebaDict.h>
#include <annotation/FreqDictionary.h>
#include <annotation/JieBa.h>
#include <annotation/Tokenizer.h>
#include <annotation/TokenizerChi.h>
#include <annotation/TokenizerJpn.h>
#include <database/CardPack.h>
#include <database/CardPackDB.h>
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
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>
#include <utils/format.h>

#include <algorithm>
#include <boost/di.hpp>
#include <filesystem>
#include <memory>
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

void adaptJiebaDictionaries(const std::shared_ptr<database::WordDB>& wordDB)
{
    auto adaptDictionary = annotation::AdaptJiebaDict{std::dynamic_pointer_cast<const dictionary::DictionaryChi>(wordDB->getDictionary())};
    adaptDictionary.load(annotation::AdaptJiebaDict::dict_in_path);
    adaptDictionary.merge();
    adaptDictionary.save(annotation::AdaptJiebaDict::dict_out_path);
    adaptDictionary.saveUserDict();
    auto adaptIdf = annotation::AdaptJiebaDict{std::dynamic_pointer_cast<const dictionary::DictionaryChi>(wordDB->getDictionary())};
    adaptIdf.load(annotation::AdaptJiebaDict::idf_in_path);
    adaptIdf.merge();
    adaptIdf.save(annotation::AdaptJiebaDict::idf_out_path);
}

auto main() -> int
{
    auto injectorJpn = boost::di::make_injector(
            boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
            boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerJpn>(),
            boost::di::bind<dictionary::Dictionary>.to<dictionary::DictionaryJpn>(),
            boost::di::bind<Language>.to(Language::japanese));

    auto db = injectorJpn.create<std::shared_ptr<sr::DataBase>>();
    auto dictionary = db->getWordDB()->getDictionary();
    auto dictionaryJpn = std::dynamic_pointer_cast<const dictionary::DictionaryJpn>(dictionary);

    // auto list1 = {"人名", "名詞", "固有名詞", "人名", "一般"};
    // 人名 -
    //     g: person's name
    // 名詞 -
    //     g: noun
    // 固有名詞 -
    //     g: proper noun
    // 人名 -
    //     g: person's name
    // 一般 -
    //     g: common

    // auto list = {"他", "感動詞", "フィラー"};
    // 他 -
    //     g: other (esp. people and abstract matters)
    // 感動詞 -
    //     かんどうし
    //     g: interjection
    // フィラー -(r)
    //     g: filler

    // auto list = {"他", "感動詞", "一般"};
    // 他 -
    //     g: other (esp. people and abstract matters)
    // 感動詞 -
    //     g: interjection
    // 一般 -
    //     g: common

    // auto list = {"他", "接続詞"};
    // 他 -
    //     g: other (esp. people and abstract matters)
    // 接続詞 -
    //     g: conjunction

    // auto list = {"体", "代名詞"};
    // 体 -
    //     g: body
    // 代名詞 -
    //     g: pattern
    //     g: pronoun
    //     g: synonym
    //     pos: noun

    auto list = {  "体","名詞","助動詞語幹"};

    // auto list = {  "体","名詞","普通名詞","サ変可能"};

    // auto list = {  "体","名詞","普通名詞","サ変形状詞可能"};

    // auto list = {  "体","名詞","普通名詞","一般"};

    // auto list = {  "体","名詞","普通名詞","副詞可能"};

    // auto list = {  "体","名詞","普通名詞","助数詞可能"};

    // auto list = {  "体","名詞","普通名詞","形状詞可能"};

    // auto list = {  "係助","助詞","係助詞"};

    // auto list = {  "副助","助詞","副助詞"};

    // auto list = {  "助動","助動詞"};

    // auto list = {  "助動","形状詞","助動詞語幹"};

    // auto list = {  "助数","接尾辞","名詞的","助数詞"};

    // auto list = {  "名","名詞","固有名詞","人名","名"};

    // auto list = {  "固有名","名詞","固有名詞","一般"};

    // auto list = {  "国","名詞","固有名詞","地名","国"};

    // auto list = {  "地名","名詞","固有名詞","地名","一般"};

    // auto list = {  "姓","名詞","固有名詞","人名","姓"};

    // auto list = {  "接助","助詞","接続助詞"};

    // auto list = {  "接尾体","接尾辞","名詞的","サ変可能"};

    // auto list = {  "接尾体","接尾辞","名詞的","一般"};

    // auto list = {  "接尾体","接尾辞","名詞的","副詞可能"};

    // auto list = {  "接尾用","接尾辞","動詞的"};

    // auto list = {  "接尾相","接尾辞","形容詞的"};

    // auto list = {  "接尾相","接尾辞","形状詞的"};

    // auto list = {  "接頭","接頭辞"};

    // auto list = {  "数","名詞","数詞"};

    // auto list = {  "格助","助詞","格助詞"};

    // auto list = {  "準助","助詞","準体助詞"};

    // auto list = {  "用","動詞","一般"};

    // auto list = {  "用","動詞","非自立可能"};

    // auto list = {  "相","副詞"};

    // auto list = {  "相","形容詞","一般"};

    // auto list = {  "相","形容詞","非自立可能"};

    // auto list = {  "相","形状詞","タリ"};

    // auto list = {  "相","形状詞","一般"};

    // auto list = {  "相","連体詞"};

    // auto list = {  "終助","助詞","終助詞"};

    // auto list = {  "補助","空白"};

    // auto list = {  "補助","補助記号","一般"};

    // auto list = {  "補助","補助記号","句点"};

    // auto list = {  "補助","補助記号","括弧閉"};

    // auto list = {  "補助","補助記号","括弧開"};

    // auto list = {  "補助","補助記号","読点"};

    // auto list = {  "補助","補助記号","ＡＡ","一般"};

    // auto list = {  "補助","補助記号","ＡＡ","顔文字"};

    // auto list = {  "記号","記号","一般"};

    // auto list = {  "記号","記号","文字"};

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

    // auto treeWalker = sr::ITreeWalker::createTreeWalker(std::move(db));
    // auto& cardMeta = treeWalker->getNextCardChoice();
    // if (cardMeta.getCardId() == 0) {
    //     spdlog::info("No card found!");
    //     return 0;
    // }
    // auto cardId = cardMeta.getCardId();
    // spdlog::info("show cardId: {}, size: {}", cardId, cardMeta.getRelevantEase().size());

    return 0;
}
