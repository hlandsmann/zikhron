#pragma once

#include "DataBase.h"
#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <misc/Identifier.h>

#include <map>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>

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

    using VocableIds_vt = std::vector<VocableId>;
    using Id_Ease_vt = std::map<VocableId, Ease>;
    using CardInformation = std::tuple<std::optional<std::unique_ptr<Card>>, VocableIds_vt, Id_Ease_vt>;

    virtual auto getNextCardChoice(std::optional<CardId> preferedCardId = {}) -> CardInformation = 0;
    virtual void setEaseLastCard(const Id_Ease_vt& id_ease) = 0;
    virtual void saveProgress() const = 0;
    // virtual auto AddVocableChoice(VocableId vocId, VocableId vocIdOldChoice, VocableId vocIdNewChoice)
    //         -> CardInformation = 0;
    // virtual auto AddAnnotation(const ZH_Annotator::Combination& combination,
    //                            const std::vector<utl::CharU8>& characterSequence)
    //         -> CardInformation = 0;

    static auto createTreeWalker(std::shared_ptr<DataBase>) -> std::unique_ptr<ITreeWalker>;
};

} // namespace sr
