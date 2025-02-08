#pragma once
#include <annotation/Ease.h>
#include <database/SpacedRepetitionData.h>
#include <database/SrsWeights.h>

#include <cmath>

namespace sr {

class Scheduler
{
    using SpacedRepetitionData = database::SpacedRepetitionData;

public:
    Scheduler() = default;
    [[nodiscard]] auto review(const SpacedRepetitionData& srd, Rating rating) const -> SpacedRepetitionData;

private:
    [[nodiscard]] auto getRetrievability(const SpacedRepetitionData& srd) const -> double;
    [[nodiscard]] auto nextDifficulty(double difficulty, double retrievability, Rating rating)const -> double;
    [[nodiscard]] auto nextForgetStability(double difficulty, double stability, double retrievability, Rating rating)const -> double;
    [[nodiscard]] auto nextRecallStability(double difficulty, double stability, double retrievability, Rating rating)const -> double;
    [[nodiscard]] auto nextStability(double difficulty, double stability, double retrievability, Rating rating)const -> double;
    [[nodiscard]] auto nextInterval(double stability) const -> double;
    double decay = -0.5;
    double factor = std::pow(0.9, (1. / decay)) - 1;
    double maxInterval = 800.0;
    database::SrsWeights weights = database::defaultSrsWeights;

    double desiredRetention = 0.9;
};

} // namespace sr
