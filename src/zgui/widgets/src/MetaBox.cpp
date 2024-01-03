#include <Box.h>
#include <Grid.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>

namespace widget {
template<class BoxImpl>
MetaBox<BoxImpl>::MetaBox(const WidgetInit& init)
    : Widget{init}
{}
template class MetaBox<Box>;
} // namespace widget
