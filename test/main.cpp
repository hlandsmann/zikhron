#include <annotation/AdaptJiebaDict.h>
#include <annotation/FreqDictionary.h>
#include <annotation/JieBa.h>
#include <annotation/Tokenizer.h>
#include <annotation/TokenizerChi.h>
#include <database/CardPack.h>
#include <database/CardPackDB.h>
#include <database/TokenizationChoiceDB.h>
#include <database/TokenizationChoiceDbChi.h>
#include <database/WordDB.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spaced_repetition/DataBase.h>
#include <spaced_repetition/ITreeWalker.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>

#include <boost/di.hpp>
#include <filesystem>
#include <memory>
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
    auto injector = boost::di::make_injector(
            boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
            boost::di::bind<database::TokenizationChoiceDB>.to<database::TokenizationChoiceDbChi>(),
            boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerChi>());

    auto db = injector.create<std::shared_ptr<sr::DataBase>>();
    auto treeWalker = sr::ITreeWalker::createTreeWalker(std::move(db));
    auto& cardMeta = treeWalker->getNextCardChoice();
    if (cardMeta.getCardId() == 0) {
        spdlog::info("No card found!");
        return 0;
    }
    auto cardId = cardMeta.getCardId();
    spdlog::info("show cardId: {}, size: {}", cardId, cardMeta.getRelevantEase().size());

    return 0;
}
