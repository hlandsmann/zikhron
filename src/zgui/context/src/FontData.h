#pragma once

#include <imgui.h>

#include <vector>
[[nodiscard]] auto getChineseGlyphRanges() -> std::vector<ImWchar>;
[[nodiscard]] auto getJapaneseGlyphRanges() -> std::vector<ImWchar>;
