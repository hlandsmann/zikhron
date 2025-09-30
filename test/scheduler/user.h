#pragma once

#include <map>
#include <random>

struct UserReviewData
{
    double difficulty{};
    double memory{};
    double dayReviewed{};
    int failCount{};
    int reviewCount{};
    bool justFailed{};
};

class User
{
public:
    auto review(int id, double day, bool shouldLog = true) -> bool;
    [[nodiscard]] auto getProb(int id, double day) const -> double;
    [[nodiscard]] auto getOptimalInterval(int id) const -> int;

private:
    auto firstReview(int id, double day) -> bool;
    static auto exponentialDecay(double high, double low, double delta, double val) -> double;
    auto genUserReviewData() -> UserReviewData;
    std::map<int, UserReviewData> userReviewData;

    std::random_device rd;
    std::mt19937 mt19937{rd()};

    int maxFailCount = 0;
};
