#pragma once
#include "CardMeta.h"
#include "DataBase.h"

#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <misc/Identifier.h>

#include <map>
#include <memory>
#include <optional>

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
    virtual void setEaseLastCard(const Id_Ease_vt& id_ease) = 0;
    virtual auto getNextCardChoice(std::optional<CardId> preferedCardId = {}) -> CardMeta& = 0;
    virtual auto getLastCard() -> CardMeta& = 0;

    static auto createTreeWalker(std::shared_ptr<DataBase>) -> std::unique_ptr<ITreeWalker>;
};

} // namespace sr
