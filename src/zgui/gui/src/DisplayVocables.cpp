#include "DisplayVocables.h"

#include "DisplayVocables_chi.h"
#include "DisplayVocables_jpn.h"

#include <imgui.h>

#include <algorithm>
#include <cstddef>

namespace gui {

template<typename Self>
void DisplayVocables::ratingByKeyMoveEmphasis(this Self&& self)
{
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_J)) {
        if (++self.emphasizeIndex == static_cast<int>(self.ratings.size())) {
            self.emphasizeIndex = 0;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_K)) {
        if (--self.emphasizeIndex < 0) {
            self.emphasizeIndex = static_cast<int>(self.ratings.size()) - 1;
        }
    }
}

template<typename Self>
auto DisplayVocables::ratingByKeyToggle(this Self&& self, int currentIndex) -> Rating
{
    Rating rating = self.ratings.at(static_cast<std::size_t>(currentIndex));
    if (std::max(0, self.emphasizeIndex) != currentIndex) {
        return rating;
    }

    bool pass = ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_L);
    bool fail = ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_H);

    if (!(fail || pass)) {
        self.keyPressed = false;
        return rating;
    }
    if (self.keyPressed) {
        return rating;
    }
    self.keyPressed = true;

    if (pass) {
        rating = Rating::pass;
    }
    if (fail) {
        rating = Rating::fail;
    }

    self.emphasizeIndex = std::clamp(self.emphasizeIndex, 0, static_cast<int>(self.ratings.size()) - 1);
    if (++self.emphasizeIndex == static_cast<int>(self.ratings.size())) {
        self.emphasizeIndex = 0;
    }
    return rating;
}

template void DisplayVocables::ratingByKeyMoveEmphasis<DisplayVocables_chi&>(DisplayVocables_chi&);
template void DisplayVocables::ratingByKeyMoveEmphasis<DisplayVocables_jpn&>(DisplayVocables_jpn&);
template auto DisplayVocables::ratingByKeyToggle<DisplayVocables_chi&>(DisplayVocables_chi&, int) -> Rating;
template auto DisplayVocables::ratingByKeyToggle<DisplayVocables_jpn&>(DisplayVocables_jpn&, int) -> Rating;
} // namespace gui
