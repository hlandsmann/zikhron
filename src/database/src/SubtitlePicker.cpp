#include "SubtitlePicker.h"

#include "IdGenerator.h"
#include "Subtitle.h"

#include <misc/Identifier.h>

#include <memory>
#include <utility>

namespace database {
SubtitlePicker::SubtitlePicker(std::shared_ptr<Subtitle> _subtitle,
                               PackId _videoId,
                               std::shared_ptr<CardIdGenerator> _cardIdGenerator)
    : subtitle{std::move(_subtitle)}
    , videoId{_videoId}
    , cardIdGenerator{std::move(_cardIdGenerator)}

{}

} // namespace database
