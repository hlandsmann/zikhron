#include "annotation/Word.h"

#include <spaced_repetition/VocableProgress.h>

#include <string>
#include <utility>

namespace annotation {
Word::Word(std::string _key)
    : key{std::move(_key)}
{}

} // namespace annotation
