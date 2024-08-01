#pragma once

#include <misc/Identifier.h>

namespace database {
class CardIdGenerator
{
public:
    CardIdGenerator() = default;
    virtual ~CardIdGenerator() = default;
    CardIdGenerator(const CardIdGenerator&) = delete;
    CardIdGenerator(CardIdGenerator&&) = delete;
    auto operator=(const CardIdGenerator&) -> CardIdGenerator& = delete;
    auto operator=(CardIdGenerator&&) -> CardIdGenerator& = delete;

    auto getNext() -> CardId;
private:
    CardId cardId{};
};


} // namespace database
