#include "DisplayAnnotation.h"

#include <annotation/Tokenizer.h>
#include <widgets/Layer.h>

#include <memory>
#include <utility>
#include <vector>

namespace gui {

DisplayAnnotation::DisplayAnnotation(std::shared_ptr<widget::Layer> _layer, std::vector<annotation::Alternative> _alternatives)
    : layer{std::move(_layer)}
    , alternatives{std::move(_alternatives)}
{}
} // namespace gui
