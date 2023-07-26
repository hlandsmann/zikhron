#include <TreeWalker.h>

#include <spdlog/spdlog.h>
#include <algorithm>
#include <memory>
#include <ranges>
#include "DataBase_deprecated.h"

namespace ranges = std::ranges;

namespace {
void walk(const SR_DataBase& db) {
    const auto& id_cardSR = db.Id_cardMeta();
    spdlog::info("Number of cards: {}", id_cardSR.size());
    for(const auto& m: id_cardSR | std::views::reverse | std::views::values | std::views::take(10)){
      spdlog::info("Number of vocs: {}", m.vocableIds.size());
    }
}

}  // namespace

TreeWalker::TreeWalker(std::shared_ptr<CardDB> cardDB_in,
                       std::shared_ptr<ZH_Dictionary> zh_dictionary_in)
    : cardDB(std::move(cardDB_in))
    , zh_dictionary(std::move(zh_dictionary_in))
    , sr_db(cardDB, zh_dictionary) {
    walk(sr_db);
}
