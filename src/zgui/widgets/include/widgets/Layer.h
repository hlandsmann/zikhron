#pragma once
#include "Widget.h"
namespace widget {
class Layer : public Widget
{
public:
    void setup();
    Layer(const WidgetInit& init);
};

} // namespace widget