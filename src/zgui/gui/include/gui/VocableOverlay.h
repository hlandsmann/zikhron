#pragma once

namespace gui {
class VocableOverlay
{
public:
    VocableOverlay() = default;
    virtual ~VocableOverlay() = default;
    VocableOverlay(const VocableOverlay&) = delete;
    VocableOverlay(VocableOverlay&&) = delete;
    auto operator=(const VocableOverlay&) -> VocableOverlay& = delete;
    auto operator=(VocableOverlay&&) -> VocableOverlay& = delete;

    virtual void draw() = 0;
    [[nodiscard]] virtual auto shouldClose() const -> bool = 0;
    [[nodiscard]] virtual auto configured() const -> bool = 0;
};
} // namespace gui
