#include <TreeWalker.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <memory>
#include <ranges>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace {
void walk(const std::shared_ptr<WalkableData>& walkableData)
{
    const auto& cards = walkableData->Cards();
    const auto& vocables = walkableData->Vocables();
    int cardIndex = 0;
    for (const auto& card : cards) {
        spdlog::info("Card: {}, activeVocs:{}", cardIndex++, walkableData->timingAndNVocables(card).nVocables);
    }
}

} // namespace

TreeWalker::TreeWalker(std::shared_ptr<WalkableData> _walkableData)
    : walkableData{std::move(_walkableData)}
{
    walk(walkableData);
}
