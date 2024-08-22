#pragma once
#include "Subtitle.h"

#include <memory>
#include <optional>
#include <string>

namespace database {

class Translation
{
public:
    Translation(std::shared_ptr<Subtitle> subtitle);
    [[nodiscard]] auto get(double beginTimeStamp, double endTimeStamp) -> std::optional<std::string>;

private:
    std::shared_ptr<Subtitle> subtitle;

    double beginTimeStamp{};
    double endTimeStamp{};
    std::optional<std::string> text;
};

} // namespace database
