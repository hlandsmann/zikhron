#include "Track.h"

#include "CardPackDB.h"
#include "VideoPackDB.h"

#include <memory>
#include <utility>

namespace database {

Track::Track(TrackMedia _medium)
    : medium{std::move(_medium)}
{
}
} // namespace database
