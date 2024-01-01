#pragma once
#include "Drop.h"


namespace context {
class WidgetIdGenerator
{
public:
    WidgetIdGenerator() = default;
    auto getNextId() -> int;

private:
    int id = 0;
};

class WidgetIdDrop : public Drop<WidgetIdDrop>
{
public:
    WidgetIdDrop(int widgetId);

private:
    friend class Drop<WidgetIdDrop>;
    static void pop();
};
} // namespace context
