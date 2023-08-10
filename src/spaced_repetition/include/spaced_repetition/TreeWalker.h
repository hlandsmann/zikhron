#pragma once

#include <memory>
#include "WalkableData.h"

class TreeWalker {
public:
    TreeWalker(std::shared_ptr<WalkableData>);

private:
    std::shared_ptr<WalkableData> walkableData;
};
