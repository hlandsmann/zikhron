#pragma once
#include <utils/spdlog.h>
enum CardId : unsigned;
enum VocableId : unsigned;
enum ColorId : unsigned;

template<>
struct fmt::formatter<CardId> : fmt::formatter<unsigned>
{};
template<>
struct fmt::formatter<VocableId> : fmt::formatter<unsigned>
{};
template<>
struct fmt::formatter<ColorId> : fmt::formatter<unsigned>
{};
