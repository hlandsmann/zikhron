#include <TreeWalker.h>

#include <spdlog/spdlog.h>
#include <algorithm>
#include <memory>
#include "DataBase.h"

namespace {
void walk(const SR_DataBase& db) {
    const auto& id_cardSR = db.Id_cardSR();
    spdlog::info("Number of cards: {}", id_cardSR.size());
}

}  // namespace

TreeWalker::TreeWalker(std::shared_ptr<CardDB> cardDB_in,
                       std::shared_ptr<ZH_Dictionary> zh_dictionary_in)
    : cardDB(std::move(cardDB_in))
    , zh_dictionary(std::move(zh_dictionary_in))
    , sr_db(cardDB, zh_dictionary) {
    walk(sr_db);
}
