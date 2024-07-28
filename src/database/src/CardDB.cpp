#include "CardDB.h"

#include "CardPackDB.h"
#include "TokenizationChoiceDB.h"
#include "VideoPackDB.h"
#include "WordDB.h"

#include <misc/Config.h>

#include <memory>
#include <utility>

namespace database {
CardDB::CardDB(std::shared_ptr<zikhron::Config> _config,
               std::shared_ptr<WordDB> _wordDB,
               std::shared_ptr<CardPackDB> _cardPackDB,
               std::shared_ptr<VideoPackDB> _videoPackDB,
               std::shared_ptr<TokenizationChoiceDB> _tokenizationChoiceDB)
    : config{std::move(_config)}
    , wordDB{std::move(_wordDB)}
    , cardPackDB{std::move(_cardPackDB)}
    , videoPackDB{std::move(_videoPackDB)}
    , tokenizationChoiceDB{std::move(_tokenizationChoiceDB)}
{}

} // namespace database
