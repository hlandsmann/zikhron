#pragma once
#include "Drop.h"

namespace context {

enum WidgetId : unsigned;

class WidgetIdGenerator
{
public:
    WidgetIdGenerator() = default;
    auto getNextId() -> WidgetId;

private:
    WidgetId id{};
};

class WidgetIdDrop : public Drop<WidgetIdDrop>
{
public:
    WidgetIdDrop(WidgetId widgetId);

private:
    friend class Drop<WidgetIdDrop>;
    static void pop();
};
} // namespace context
