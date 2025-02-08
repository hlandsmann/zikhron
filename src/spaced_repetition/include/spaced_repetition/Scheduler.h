#pragma once
#include <annotation/Ease.h>
#include <database/SpacedRepetitionData.h>
#include <database/SrsWeights.h>

#include <chrono>
#include <cmath>
#include <ratio>

namespace sr {

class Scheduler
{
    using SpacedRepetitionData = database::SpacedRepetitionData;
    using StudyState = database::StudyState;

public:
    Scheduler() = default;
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

    [[nodiscard]] auto getRetrievability(const SpacedRepetitionData& srd) const -> double;
    [[nodiscard]] auto initialStability(Rating rating) const -> double;
    [[nodiscard]] auto nextForgetStability(double difficulty,
                                           double stability,
                                           double retrievability,
                                           Rating rating) const -> double;
    [[nodiscard]] auto nextRecallStability(double difficulty,
                                           double stability,
                                           double retrievability,
                                           Rating rating) const -> double;
    [[nodiscard]] auto nextStability(double difficulty,
                                     double stability,
                                     double retrievability,
                                     Rating rating) const -> double;
    [[nodiscard]] auto nextInterval(double stability) const -> std::chrono::days;
    [[nodiscard]] auto nextDifficulty(double difficulty,
                                      Rating rating) const -> double;
    [[nodiscard]] auto initialDifficulty(Rating rating) const -> double;
    double decay = -0.5;
    double factor = std::pow(0.9, (1. / decay)) - 1;
    double maxInterval = 800.0;
    database::SrsWeights weights = database::defaultSrsWeights;

    double desiredRetention = 0.9;

    // std::chrono::duration<double, std::chrono::minutes> x=10m;
    std::chrono::minutes learnAgainTime{1};
    std::chrono::minutes learnHardTime{6};
    std::chrono::minutes learnGoodTime{10};
    std::chrono::minutes relearnAgainTime{10};
    std::chrono::minutes relearnHardTime{15};
};

} // namespace sr
