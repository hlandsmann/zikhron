#pragma once
#include "Widget.h"

#include <imgui.h>

namespace widget {
enum class ib {
    cards,
};

class Button : public Widget<Button>
{
};

}; // namespace widget
