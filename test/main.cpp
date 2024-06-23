#include <annotation/AdaptJiebaDict.h>
#include <annotation/CardPack.h>
#include <annotation/CardPackDB.h>
#include <annotation/FreqDictionary.h>
#include <annotation/JieBa.h>
#include <annotation/WordDB.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spaced_repetition/DataBase.h>
#include <spaced_repetition/ITreeWalker.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>

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

void adaptJiebaDictionaries(const std::shared_ptr<annotation::WordDB>& wordDB)
{
    auto adaptDictionary = annotation::AdaptJiebaDict{wordDB->getDictionary()};
    adaptDictionary.load(annotation::AdaptJiebaDict::dict_in_path);
    adaptDictionary.merge();
    adaptDictionary.save(annotation::AdaptJiebaDict::dict_out_path);
    adaptDictionary.saveUserDict();
    auto adaptIdf = annotation::AdaptJiebaDict{wordDB->getDictionary()};
    adaptIdf.load(annotation::AdaptJiebaDict::idf_in_path);
    adaptIdf.merge();
    adaptIdf.save(annotation::AdaptJiebaDict::idf_out_path);
}

auto main() -> int
{
    auto zikhron_cfg = get_zikhron_cfg();
    auto db = std::make_shared<sr::DataBase>(zikhron_cfg);
    auto treeWalker = sr::ITreeWalker::createTreeWalker(std::move(db));
    auto& cardMeta = treeWalker->getNextCardChoice();
    if (cardMeta.Id() == 0) {
        spdlog::info("No card found!");
        return 0;
    }
    auto cardId = cardMeta.Id();
    spdlog::info("show cardId: {}, size: {}", cardId, cardMeta.getRelevantEase().size());

    return 0;
}
