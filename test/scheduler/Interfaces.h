#pragma once

namespace scheduler {
struct Card
{
    int id;
    double true_difficulty; // Ground truth difficulty (0 to 1)
    double memory{};
    int review_count;
    bool is_new;
    double srs_difficulty; // SRS-estimated difficulty
    double stability;      // For FSRS
    double interval;       // Current interval in days
    double next_review;    // Day for next review
    double seen;
    // std::vector<bool> pass_history; // true for pass, false for fail
};

class SRS
{
public:
    virtual void schedule_card(Card& card, bool pass, int day) = 0;

    virtual void reschedule_card(Card& /* card */) {}

    virtual auto shouldReschedule() -> bool { return false; }

    virtual void rescheduleInternal() {};

    virtual void update_difficulty(Card& /* card */, bool /* pass */)
    {
        // Simple difficulty update based on pass/fail history
        // double pass_rate = card.pass_history.empty() ? 0.5 : std::accumulate(card.pass_history.begin(), card.pass_history.end(), 0.0) / card.pass_history.size();
        // card.srs_difficulty = 1.0 - pass_rate; // Higher difficulty for lower pass rate
    }

    virtual void printWeights() {}

    virtual ~SRS() = default;
};
} // namespace scheduler
