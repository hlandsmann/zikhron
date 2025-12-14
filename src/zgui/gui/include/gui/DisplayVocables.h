#pragma once
#include <misc/Identifier.h>
#include <spaced_repetition/Scheduler.h>

#include <map>

namespace gui {

class DisplayVocables
{
public:
    DisplayVocables() = default;
    virtual ~DisplayVocables() = default;
    DisplayVocables(const DisplayVocables&) = delete;
    DisplayVocables(DisplayVocables&&) = delete;
    auto operator=(const DisplayVocables&) -> DisplayVocables& = delete;
    auto operator=(DisplayVocables&&) -> DisplayVocables& = delete;

    using Rating = sr::Rating;
    using VocableId_Rating = std::map<VocableId, Rating>;

    virtual void draw() = 0;
    virtual void reload() = 0;
    [[nodiscard]] virtual auto getRatedVocables() const -> VocableId_Rating = 0;

protected:
    template<typename Self>
    void ratingByKeyMoveEmphasis(this Self&& self);
    template<typename Self>
    auto ratingByKeyToggle(this Self&& self, int currentIndex) -> Rating;

private:
    bool keyPressed{false};
};

} // namespace gui
