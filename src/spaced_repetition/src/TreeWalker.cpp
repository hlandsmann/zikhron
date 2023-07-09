#include <TreeWalker.h>

#include <spdlog/spdlog.h>
TreeWalker::TreeWalker(const std::shared_ptr<CardDB>& cardDB_in,
                       const std::shared_ptr<ZH_Dictionary>& zh_dictionary_in)
    : cardDB(cardDB_in), zh_dictionary(zh_dictionary_in), sr_db(cardDB, zh_dictionary) {
}
