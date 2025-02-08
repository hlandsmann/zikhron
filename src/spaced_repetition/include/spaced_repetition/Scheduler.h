#pragma once
#include <database/SpacedRepetitionData.h>
#include <database/SrsWeights.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <ratio>
#include <utility>

namespace sr {

enum class Rating {
    fail,
    pass,
    familiar,
};

class Scheduler
{
    using SpacedRepetitionData = database::SpacedRepetitionData;
    using StudyState = database::StudyState;

public:
    Scheduler();
    [[nodiscard]] auto review(const SpacedRepetitionData& srd,
                              Rating rating) const -> SpacedRepetitionData;
    [[nodiscard]] auto getIntervalDays(const SpacedRepetitionData& srd) const -> std::chrono::days;

private:
    // using Days = std::chrono::duration<double, std::ratio<86400>>;
    [[nodiscard]] auto reviewStateNewWord(const SpacedRepetitionData& srd,
                                          Rating rating) const -> SpacedRepetitionData;
    [[nodiscard]] auto reviewStateLearning(const SpacedRepetitionData& srd,
                                           Rating rating) const -> SpacedRepetitionData;
    [[nodiscard]] auto reviewStateReview(const SpacedRepetitionData& srd,
                                         Rating rating) const -> SpacedRepetitionData;
    [[nodiscard]] auto reviewStateRelearning(const SpacedRepetitionData& srd,
                                             Rating rating) const -> SpacedRepetitionData;

    [[nodiscard]] auto initialStability(Rating rating) const -> double;
    [[nodiscard]] auto initialEase(Rating rating) const -> double;
    [[nodiscard]] auto nextStability(double stability,
                                     double ease,
                                     Rating rating) const -> double;
    [[nodiscard]] auto getStep(double ease) const -> double;
    [[nodiscard]] auto failStabilityAndEase(const SpacedRepetitionData& srd) const -> std::pair<double, double>;
    [[nodiscard]] auto stabilityFromInterval(double interval, double ease) const -> double;
    [[nodiscard]] auto getBin(double stability, double ease) const -> double;
    [[nodiscard]] auto getInterval(double stability, double ease) const -> double;
    [[nodiscard]] auto getIntervalDays(double stability, double ease) const -> std::chrono::days;

    [[nodiscard]] static auto increaseEase(double ease, double speed) -> double;
    [[nodiscard]] static auto decreaseEase(double ease, double speed) -> double;
    [[nodiscard]] static auto increaseEaseTowardsNeutral(double ease, double speed) -> std::pair<double, double>;
    [[nodiscard]] static auto decreaseEaseTowardsNeutral(double ease, double speed) -> std::pair<double, double>;
    [[nodiscard]] auto nextEase(double stability,
                                double ease) const -> double;
    [[nodiscard]] auto shiftBackward(const SpacedRepetitionData& srd,
                                     double stability,
                                     double ease,
                                     Rating rating) const -> int;
    [[nodiscard]] auto shiftForward(const SpacedRepetitionData& srd,
                                    double stability,
                                    double ease,
                                    Rating rating) const -> int;
    void initBins();

    double decay = -0.5;
    // double factor = 19. / 81.; // std::pow(0.9, (1. / decay)) - 1;
    double maxInterval = 800.0;
    database::SrsWeights weights = database::defaultSrsWeights;

    double desiredRetention = 0.9;

    // std::chrono::duration<double, std::chrono::minutes> x=10m;
    std::chrono::minutes learnAgainTime{1};
    std::chrono::minutes learnFirstStepTime{6};
    std::chrono::minutes learnSecondStepTime{10};
    std::chrono::minutes relearnAgainTime{10};

    constexpr static double familiarStability = 3;
    constexpr static double familiarEase = 2;

    constexpr static double speedChange = 0.2;
    constexpr static double speedStabilize = 0.13;

    static constexpr std::size_t numberOfBins = 24;
    std::array<double, numberOfBins> binsHard{};
    std::array<double, numberOfBins> binsGood{};
    std::array<double, numberOfBins> binsEasy{};
};

} // namespace sr
