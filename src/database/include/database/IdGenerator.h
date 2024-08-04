#pragma once

#include <misc/Identifier.h>

namespace database {
template<class ID>
class IdGenerator
{
public:
    IdGenerator() = default;
    virtual ~IdGenerator() = default;
    IdGenerator(const IdGenerator&) = delete;
    IdGenerator(IdGenerator&&) = delete;
    auto operator=(const IdGenerator&) -> IdGenerator& = delete;
    auto operator=(IdGenerator&&) -> IdGenerator& = delete;

    auto getNext() -> ID
    {
        id = static_cast<ID>(static_cast<unsigned>(id) + 1);
        return id;
    }

private:
    ID id{};
};

using CardIdGenerator = IdGenerator<CardId>;
using PackIdGenerator = IdGenerator<PackId>;

} // namespace database
