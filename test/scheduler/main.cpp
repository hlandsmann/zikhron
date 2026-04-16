#include "Interfaces.h"
#include "LSTM.h"
#include "SRS_data.h"
#include "ThompsonScheduler.h"
#include "scheduler.h"
#include "user.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

// Random number generator setup
std::random_device rd;
std::mt19937 gen(rd());

// Card structure
using Card = scheduler::Card;
using SRS = scheduler::SRS;

Card create_card(int id, double day)
{
    Card card;
    card.id = id;
    std::uniform_real_distribution<> dist(0.0, 1.0);
    card.true_difficulty = dist(gen); // Random difficulty between 0 and 1
    card.memory = 0;
    card.review_count = 0;
    card.is_new = true;
    card.srs_difficulty = 0.5; // Initial estimate
    card.stability = 2.0;      // Initial stability for FSRS
    card.interval = 1.0;
    card.next_review = day;
    card.seen = 0;
    return card;
}

// };

// Base SRS class

// SuperMemo3 implementation
class SuperMemo3 : public SRS
{
public:
    void schedule_card(Card& card, bool pass, int day) override
    {
        update_difficulty(card, pass);
        double easiness = 2.5 - card.srs_difficulty; // Easiness factor
        easiness = std::max(1.3, easiness);
        if (card.is_new) {
            card.interval = 1.0;
            card.is_new = false;
        } else {
            if (pass) {
                if (card.review_count == 1)
                    card.interval = 1.0;
                else if (card.review_count == 2)
                    card.interval = 6.0;
                else
                    card.interval *= easiness;
            } else {
                card.interval = 1.0;
                easiness = std::max(1.3, easiness - 0.3);
            }
        }
        card.next_review = day + card.interval;
        card.srs_difficulty = 2.5 - easiness; // Update difficulty based on easiness
    }
};

class MyScheduler : public SRS
{
public:
    void schedule_card(Card& card, bool pass, int day) override
    {
        int nextInterval = scheduler.nextInterval(day, card.id, pass);
        card.next_review = day + nextInterval;
        card.seen = day;
    }

    void reschedule_card(Card& card) override { card.next_review = scheduler.getNextReview(card.id); };

    auto shouldReschedule() -> bool override { return scheduler.shouldReschedule(); }

    void rescheduleInternal() override { scheduler.rescheduleInternal(); };

    void clearItems() { scheduler.clearItems(); }

    void printWeights() override { scheduler.printWeights(); }

private:
    Scheduler scheduler{};
};

class MyThompsonScheduler : public SRS
{
public:
    void schedule_card(Card& card, bool pass, int day) override
    {
        int nextInterval = scheduler.nextInterval(day, card.id, pass);
        card.next_review = day + nextInterval;
        card.seen = day;
    }

    void reschedule_card(Card& card) override { card.next_review = scheduler.getNextReview(card.id); };

    auto shouldReschedule() -> bool override { return scheduler.shouldReschedule(); }

    void rescheduleInternal() override { scheduler.rescheduleInternal(); };

    void clearItems() { scheduler.clearItems(); }

    void printWeights() override { scheduler.printWeights(); }

private:
    ThompsonScheduler scheduler{};
};

// Simulation function with daily summary
auto run_simulation(SRS* srs, const std::string& srs_name) -> std::pair<int, double>
{
    spdlog::info("simulation {}: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>", srs_name);
    User user;
    std::vector<Card> cards;
    int card_id = 0;
    double total_reviews = 0;
    double successful_reviews = 0;

    std::cout << srs_name << " Daily Summaries:\n";
    std::cout << "-----------------------------------\n";
    int day = 0;
    for (day = 1; day <= 365 * 2; ++day) {
        bool cardIsPresent = false;
        // Review due cards
        int daily_reviews = 0;
        int daily_passes = 0;
        bool shouldReschedule = false;
        std::set<double> probs;
        while (true) {
            for (auto& card : cards) {
                if (card.next_review <= day) {
                    probs.insert(user.getProb(card.id, day));
                    bool pass = user.review(card.id, day);
                    srs->schedule_card(card, pass, day);
                    daily_reviews++;
                    total_reviews++;
                    if (pass) {
                        daily_passes++;
                        successful_reviews++;
                    }
                    if (card.id == 1) {
                        cardIsPresent = true;
                    }
                }
                shouldReschedule |= srs->shouldReschedule();
                // if (shouldReschedule) {
                //     // break;
                // }
            }
            if (!shouldReschedule) {
                break;
            }
            shouldReschedule = false;
            srs->rescheduleInternal();
            for (auto& card : cards) {
                srs->reschedule_card(card);
            }
        }

        // if (cardIsPresent) {
        if (day % 30 == 1) {
            // Print daily summary
            double daily_pass_rate = daily_reviews > 0 ? (daily_passes / static_cast<double>(daily_reviews)) * 100 : 0.0;
            std::cout << "Day " << day << ": Reviews = " << daily_reviews
                      << ", Passes = " << daily_passes
                      << ", Pass Rate = " << daily_pass_rate << "%\n";
            srs->printWeights();
        }

        // Add 20 new cards
        for (int i = 0; i < 20; ++i) {
            cards.push_back(create_card(card_id++, day));
            auto& card = cards.back();
            bool pass = user.review(card.id, day);
            srs->schedule_card(card, pass, day);
        }
    }

    // Calculate true retention
    double true_retention = 0.0;
    int retained_cards = 0;
    for (auto& card : cards) {
        // double effective_difficulty = card.true_difficulty * (1.0 - card.review_count * 0.05);
        // effective_difficulty = std::max(0.0, std::min(1.0, effective_difficulty));
        // double retention_prob = 1.0 - (effective_difficulty < 0.5 ? 0.1 : 0.7);
        // true_retention += retention_prob;
        // if (retention_prob > 0.5)
        //     retained_cards++;
        // auto srsData = SRS_data{};
        // srsData.id = card.id;
        bool pass = user.review(card.id, day, false);
        // bool pass = user.review_card(card, day);
        if (pass) {
            retained_cards++;
        }
    }
    true_retention = static_cast<double>(retained_cards) / static_cast<double>(cards.size());

    std::cout << "\n"
              << srs_name << " Final Results:\n";
    std::cout << "Total Cards: " << cards.size() << "\n";
    std::cout << "Total Reviews: " << total_reviews << "\n";
    std::cout << "Successful Reviews: " << successful_reviews << "\n";
    std::cout << "Overall Pass Rate: " << (successful_reviews / total_reviews) * 100 << "%\n";
    std::cout << "Average True Retention: " << true_retention * 100 << "%\n";
    std::cout << "Retained Cards (>50% retention): " << retained_cards << "\n\n";
    return {static_cast<int>(total_reviews), true_retention * 100};
}

auto run_optimalsimulation() -> std::pair<int, double>
{
    spdlog::info("optimal simulation: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    User user;
    std::vector<Card> cards;
    int card_id = 0;
    double total_reviews = 0;
    double successful_reviews = 0;

    int day = 0;
    for (day = 1; day <= 365 * 2; ++day) {
        bool cardIsPresent = false;
        // Review due cards
        int daily_reviews = 0;
        int daily_passes = 0;
        std::set<double> probs;
        for (auto& card : cards) {
            if (card.next_review <= day) {
                probs.insert(user.getProb(card.id, day));
                bool pass = user.review(card.id, day);
                card.next_review = day + user.getOptimalInterval(card.id);
                daily_reviews++;
                total_reviews++;
                if (pass) {
                    daily_passes++;
                    successful_reviews++;
                }
                if (card.id == 1) {
                    cardIsPresent = true;
                }
            }
        }

        // if (cardIsPresent) {
        if (day % 30 == 1) {
            // Print daily summary
            double daily_pass_rate = daily_reviews > 0 ? (daily_passes / static_cast<double>(daily_reviews)) * 100 : 0.0;
            std::cout << "Day " << day << ": Reviews = " << daily_reviews
                      << ", Passes = " << daily_passes
                      << ", Pass Rate = " << daily_pass_rate << "%\n";
            std::map<int, int> pcount;
            for (double p : probs) {
                pcount[static_cast<int>(p * 100)]++;
                // probstr = fmt::format("{}, {:.2F}", probstr, p);
            }
            // for (const auto [p, count] : pcount) {
            //     fmt::print("{:.2F}, {}\n", static_cast<double>(p) / 100., count);
            // }
        }

        // Add 20 new cards
        for (int i = 0; i < 20; ++i) {
            cards.push_back(create_card(card_id++, day));
            auto& card = cards.back();
            bool pass = user.review(card.id, day);
        }
    }

    // Calculate true retention
    double true_retention = 0.0;
    int retained_cards = 0;
    for (auto& card : cards) {
        bool pass = user.review(card.id, day, false);
        if (pass) {
            retained_cards++;
        }
    }
    true_retention = static_cast<double>(retained_cards) / static_cast<double>(cards.size());

    std::cout << "Total Cards: " << cards.size() << "\n";
    std::cout << "Total Reviews: " << total_reviews << "\n";
    std::cout << "Successful Reviews: " << successful_reviews << "\n";
    std::cout << "Overall Pass Rate: " << (successful_reviews / total_reviews) * 100 << "%\n";
    std::cout << "Average True Retention: " << true_retention * 100 << "%\n";
    std::cout << "Retained Cards (>50% retention): " << retained_cards << "\n\n";
    return {static_cast<int>(total_reviews), true_retention * 100};
}

auto main() -> int
{
    run_optimalsimulation();
    // Scheduler scheduler1;
    // scheduler1.test();
    // return 0;
    SuperMemo3 sm3;
    // scheduler::FSRS fsrs;
    constexpr int maxCount = 1;
    std::pair<int, double> result1{};
    std::pair<int, double> result2{};
    for (int c = 1; c <= maxCount; c++) {
        MyScheduler scheduler;

        auto tmp = run_simulation(&scheduler, "Scheduler");
        result1.first += tmp.first;
        result1.second += tmp.second;
        // scheduler.printWeights();
        scheduler.clearItems();

        tmp = run_simulation(&scheduler, "Scheduler");
        result2.first += tmp.first;
        result2.second += tmp.second;
        // scheduler.printWeights();

        // run_simulation(&fsrs, "FSRS");

        // run_simulation(&sm3, "SuperMemo3");
        spdlog::info("----------------------------------Final result1: {}, ret: {}", result1.first / c, result1.second / c);
        spdlog::info("----------------------------------Final result2: {}, ret: {}", result2.first / c, result2.second / c);
    }
    result1 = {};
    result2 = {};
    for (int c = 1; c <= maxCount; c++) {
        MyThompsonScheduler scheduler;

        auto tmp = run_simulation(&scheduler, "Thompson");
        result1.first += tmp.first;
        result1.second += tmp.second;
        // scheduler.printWeights();
        scheduler.clearItems();

        tmp = run_simulation(&scheduler, "Thompson");
        result2.first += tmp.first;
        result2.second += tmp.second;
        // scheduler.printWeights();

        // run_simulation(&fsrs, "FSRS");

        // run_simulation(&sm3, "SuperMemo3");
        spdlog::info("----------------------------------Final result1: {}, ret: {}", result1.first / c, result1.second / c);
        spdlog::info("----------------------------------Final result2: {}, ret: {}", result2.first / c, result2.second / c);
    }
}
