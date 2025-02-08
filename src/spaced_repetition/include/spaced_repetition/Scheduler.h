#pragma once
#include <annotation/Ease.h>
#include <database/SpacedRepetitionData.h>
#include <database/SrsWeights.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <ratio>
#include <utility>

namespace sr {

class Scheduler
{
    using SpacedRepetitionData = database::SpacedRepetitionData;
    using StudyState = database::StudyState;

public:
    Scheduler();
    [[nodiscard]] auto review(const SpacedRepetitionData& srd,
                              Rating rating) const -> SpacedRepetitionData;

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
    [[nodiscard]] auto nextStability(double stability,
                                     double difficulty,
                                     Rating rating) const -> double;
    [[nodiscard]] auto failStabilityAndDifficulty(const SpacedRepetitionData& srd) const -> std::pair<double, double>;
    [[nodiscard]] auto nextInterval(double stability, double difficulty) const -> double;
    [[nodiscard]] auto nextIntervalDays(double stability, double difficulty) const -> std::chrono::days;
    [[nodiscard]] auto nextDifficulty(double stability,
                                      double difficulty,
                                      Rating rating) const -> double;
    [[nodiscard]] auto initialDifficulty(Rating rating) const -> double;
    [[nodiscard]] auto shiftBackward(const SpacedRepetitionData& srd,
                                     double stability,
                                     double difficulty,
                                     Rating rating) const -> int;
    void initBins();

    double decay = -0.5;
    // double factor = 19. / 81.; // std::pow(0.9, (1. / decay)) - 1;
    double maxInterval = 800.0;
    database::SrsWeights weights = database::defaultSrsWeights;

    double desiredRetention = 0.9;

    // std::chrono::duration<double, std::chrono::minutes> x=10m;
    std::chrono::minutes learnAgainTime{1};
    std::chrono::minutes learnHardTime{6};
    std::chrono::minutes learnGoodTime{10};
    std::chrono::minutes relearnAgainTime{10};
    std::chrono::minutes relearnHardTime{15};

    static constexpr std::size_t numberOfBins = 24;
    std::array<double, numberOfBins> binsHard{};
    std::array<double, numberOfBins> binsGood{};
    std::array<double, numberOfBins> binsEasy{};
};

} // namespace sr
