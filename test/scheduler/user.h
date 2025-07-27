#pragma once
#include "SRS_data.h"

#include <map>
#include <random>

struct UserReviewData
{
    double difficulty{};
    double memory{};
};

class User
{
public:
    auto review(const SRS_data& srs_data, double day) -> bool;

private:
    static auto exponentialDecay(double high, double low, double delta, double val) -> double;
    auto genUserReviewData() -> UserReviewData;
    std::map<int, UserReviewData> userReviewData;

    std::random_device rd;
    std::mt19937 mt19937{rd()};
};
