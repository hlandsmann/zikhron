#pragma once
#include "Widget.h"
namespace widget {
template<class BoxImpl>
class MetaBox : public Widget
{
public:
    MetaBox(const WidgetInit& init);
};

} // namespace widget
