#include "DisplayAnnotation.h"

#include <annotation/TokenText.h>
#include <annotation/Tokenizer.h>
#include <widgets/Layer.h>

#include <memory>
#include <utility>
#include <vector>

namespace gui {

DisplayAnnotation::DisplayAnnotation(std::shared_ptr<widget::Layer> _layer,
                                     std::vector<annotation::Alternative> _alternatives,
                                     std::unique_ptr<annotation::TokenText> tokenText)
    : layer{std::move(_layer)}
    , alternatives{std::move(_alternatives)}
{}
} // namespace gui
