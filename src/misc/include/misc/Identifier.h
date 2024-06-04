#pragma once
#include <utils/spdlog.h>

#include <limits>
enum CardId : unsigned;
enum VocableId : unsigned;

enum ColorId : unsigned {
    shadowFontColor = std::numeric_limits<unsigned>::max() - 1,
    defaultFontColor = std::numeric_limits<unsigned>::max(),
};

template<>
struct fmt::formatter<CardId> : fmt::formatter<unsigned>
{};

template<>
struct fmt::formatter<VocableId> : fmt::formatter<unsigned>
{};

template<>
struct fmt::formatter<ColorId> : fmt::formatter<unsigned>
{};
