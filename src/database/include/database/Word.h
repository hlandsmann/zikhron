#pragma once
#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
#include <dictionary/Entry.h>
#include <misc/Identifier.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace database {
class SpacedRepetitionData;

class Word
{
public:
    Word() = default;
    Word(const Word&) = default;
    Word(Word&&) = default;
    virtual ~Word() = default;
    auto operator=(const Word&) -> Word& = default;
    auto operator=(Word&&) -> Word& = default;

    [[nodiscard]] virtual auto serialize() const -> std::string = 0;
    [[nodiscard]] virtual auto getId() const -> VocableId = 0;
    [[nodiscard]] virtual auto Key() const -> std::string = 0;
    [[nodiscard]] virtual auto getSpacedRepetitionData() const -> std::shared_ptr<SpacedRepetitionData> = 0;
};

} // namespace database
