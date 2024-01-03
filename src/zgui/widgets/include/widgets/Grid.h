#pragma once
#include "Widget.h"
namespace widget {
class Grid : public Widget
{
    friend class Widget;
    friend class Box;
    void setup();

public:
    Grid(const WidgetInit& init);
};

} // namespace widget
