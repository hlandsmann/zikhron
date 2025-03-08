#pragma once
#include "CardMeta.h"
#include "DataBase.h"

#include <annotation/Ease.h>
#include <database/CbdFwd.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <map>
#include <memory>

#include <sys/types.h>

namespace sr {
class ITreeWalker
{
public:
    ITreeWalker() = default;
    virtual ~ITreeWalker() = default;
    ITreeWalker(const ITreeWalker&) = delete;
    ITreeWalker(ITreeWalker&&) = delete;
    auto operator=(const ITreeWalker& other) -> ITreeWalker& = delete;
    auto operator=(ITreeWalker&& other) noexcept -> ITreeWalker& = delete;

    using Id_Ease_vt = std::map<VocableId, Ease>;
    // virtual void setEaseForCard(database::CardPtr cardId, const Id_Ease_vt& id_ease) = 0;
    virtual auto getNextCardChoice() -> const CardMeta& = 0;
    [[nodiscard]] virtual auto getNumberOfFailedVocables() const -> unsigned = 0;
    [[nodiscard]] virtual auto getNumberOfTodayVocables() const -> unsigned = 0;

    static auto createTreeWalker(std::shared_ptr<DataBase>) -> std::unique_ptr<ITreeWalker>;
};

} // namespace sr
