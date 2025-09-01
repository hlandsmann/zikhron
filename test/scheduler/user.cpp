#include "user.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <random>

auto User::review(int id, double day, bool shouldLog) -> bool
{
    if (!userReviewData.contains(id)) {
        return firstReview(id, day);
    }

    auto& reviewData = userReviewData[id];
    double& difficulty = reviewData.difficulty;
    double& memory = reviewData.memory;
    double interval = day - reviewData.dayReviewed;
    reviewData.dayReviewed = day;
    reviewData.reviewCount++;

    double passProbability = exponentialDecay(1., 0.9, memory, interval);
    // spdlog::info("id: {}, memory: {}, interval: {}", id, memory, interval);
    std::bernoulli_distribution bernoulli{passProbability};
    bool pass = bernoulli(mt19937);
    if (shouldLog) {
        // if (id == 1) {
        //     spdlog::info("                                           Pass: {} - {:.2f}% memory: {:.2f}, interval: {:.2f}, difficulty: {:.2f}",
        //                  pass, passProbability, memory, interval, difficulty);
        // }
        // if (maxFailCount == reviewData.failCount) {

        // if (reviewData.failCount == 4 && interval > 8 && interval < 32) {
        //     spdlog::info("           fc: {} , rc: {}- {:.2f}% memory: {:.2f}, interval: {:.2f}, difficulty: {:.2f}",
        //                  reviewData.failCount, reviewData.reviewCount, passProbability, memory, interval, difficulty);
        // }
        // if (reviewData.justFailed && interval > 4) {
        //     spdlog::info("                   failcount: {} - {:.2f}% memory: {:.2f}, interval: {:.2f}, difficulty: {:.2f}",
        //                  reviewData.failCount, passProbability, memory, interval, difficulty);
        // }
    }
    reviewData.justFailed = !pass;
    if (pass) {
        memory += std::min(memory, interval) * (1.5 - difficulty);
        // memory = std::min(memory, interval) * (2.5 - difficulty);
        // difficulty = std::max(difficulty - 0.05, 0.);

    } else {
        memory = std::clamp(std::log(memory) * (1.5 - difficulty), 0.5, std::max(1., memory * 0.8));
        // difficulty = std::max(difficulty - 0.09, 0.);
        reviewData.failCount++;
    }

    maxFailCount = std::max(maxFailCount, reviewData.failCount);
    return pass;
}

auto User::firstReview(int id, double day) -> bool
{
    userReviewData[id] = genUserReviewData();
    auto& reviewData = userReviewData[id];

    reviewData.dayReviewed = day;
    auto& difficulty = reviewData.difficulty;
    auto& memory = reviewData.memory;

    if (difficulty < 0.1) {
        memory = 8;
        return true;
    }
    if (difficulty < 0.2) {
        memory = 4;
        return true;
    }
    memory = 1.5 - difficulty;
    return false;
}

auto User::exponentialDecay(double high, double low, double delta, double val) -> double
{
    if (delta <= 0 || high <= low || low <= 0) {
        return 0.0; // Invalid input handling
    }
    double k = std::log(high / low) / delta;
    return high * std::exp(-k * val);
}

auto User::genUserReviewData() -> UserReviewData
{
    std::uniform_real_distribution<> dist(0.0, 1.0);
    return {.difficulty = dist(mt19937), .memory = 0, .dayReviewed = 0, .failCount = 0, .reviewCount = 0, .justFailed = false};
}
