#include "Translation.h"

#include "Subtitle.h"

#include <multimedia/Subtitle.h>
#include <utils/format.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>
namespace ranges = std::ranges;
namespace views = std::views;

namespace database {
Translation::Translation(std::shared_ptr<Subtitle> _subtitle)
    : subtitle{std::move(_subtitle)}
{}

auto Translation::get(double _beginTimeStamp, double _endTimeStamp) -> std::optional<std::string>
{
    if (beginTimeStamp == _beginTimeStamp && endTimeStamp == _endTimeStamp) {
        return text;
    }

    auto translation = std::string{};
    auto snippets = std::vector<std::string>{};

    beginTimeStamp = _beginTimeStamp;
    endTimeStamp = _endTimeStamp;
    ranges::transform(subtitle->getSubTexts() | views::filter([this](const multimedia::SubText& subtext) -> bool {
                          return (beginTimeStamp
                                          == std::clamp(beginTimeStamp, subtext.getStartTimeStamp(), subtext.getEndTimeStamp())
                                  || (endTimeStamp
                                      == std::clamp(endTimeStamp, subtext.getStartTimeStamp(), subtext.getEndTimeStamp()))
                                  || (subtext.getStartTimeStamp()
                                      == std::clamp(subtext.getStartTimeStamp(), beginTimeStamp, endTimeStamp))
                                  || (subtext.getEndTimeStamp()
                                      == std::clamp(subtext.getEndTimeStamp(), beginTimeStamp, endTimeStamp)));
                      }),
                      std::back_inserter(snippets), &multimedia::SubText::text);
    translation = fmt::format("{}", fmt::join(snippets, " - "));
    if (translation.empty()) {
        text.reset();
    } else {
        text = translation.substr(0, 1024);
    }
    return text;
}

} // namespace database
