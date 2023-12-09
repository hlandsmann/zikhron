#pragma once
#include "TextToken.h"
#include "Widget.h"

#include <annotation/TokenText.h>
#include <imgui.h>

#include <string>
namespace widget {
class TextTokenSeq : public Widget<TextTokenSeq>
{
public:
    TextTokenSeq(WidgetInit init);

private:
};

} // namespace widget
