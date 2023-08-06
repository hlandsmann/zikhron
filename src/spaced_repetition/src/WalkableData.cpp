#include <WalkableData.h>
#include <spdlog/spdlog.h>

WalkableData::WalkableData(std::shared_ptr<zikhron::Config> config)
    : db{std::move(config)}
{
    fillIndexMaps();
}

void WalkableData::fillIndexMaps()
{
    for (const auto& [_, card] : db.getCards()) {
        insertVocabularyOfCard(card);
    }
}

void WalkableData::insertVocabularyOfCard(const CardDB::CardPtr& card) {}
