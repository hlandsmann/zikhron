#include "Track.h"

#include "CardPackDB.h"
#include "VideoDB.h"

#include <memory>
#include <utility>

namespace database {

Track::Track(TrackMedia _medium)
    : medium{std::move(_medium)}
{
}

Track::Track(TrackMedia _medium, CardId _cardId)
{
}
} // namespace database
