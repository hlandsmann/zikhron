#include "LSTM.h"
#include "SRS_data.h"
#include "scheduler.h"
#include "user.h"

#include <spdlog/spdlog.h>

auto _main() -> int
{
    spdlog::info("Scheduler");
    return 0;
}

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

// Random number generator setup
std::random_device rd;
std::mt19937 gen(rd());

// Card structure
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

auto exponentialDecay(double high, double low, double delta, double x) -> double
{
    if (delta <= 0 || high <= low || low <= 0) {
        return 0.0; // Invalid input handling
    }
    double k = std::log(high / low) / delta;
    return high * std::exp(-k * x);
}

// // User model
// class User_
// {
// private:
//     double fail_prob_easy;       // Base fail probability for easy cards
//     double fail_prob_difficult;  // Base fail probability for difficult cards
//     double difficulty_reduction; // Reduction in difficulty per review
// public:
//     User_()
//         : fail_prob_easy(0.1), fail_prob_difficult(0.7), difficulty_reduction(0.05) {}
//
//     auto review_card(Card& card, double day) const -> bool
//     {
//         double interval = day - card.seen;
//         card.seen = day;
//         double effective_difficulty = card.true_difficulty * (1.0 - card.review_count * difficulty_reduction);
//         effective_difficulty = std::clamp(effective_difficulty, 0.0, 1.0); // std::max(0.0, std::min(1.0, effective_difficulty));
//         // effective_difficulty = 0;
//
//         double succ_prob = exponentialDecay(0.9, 0.2,
//                                             2 + (card.memory * (2. + 1. - effective_difficulty)),
//                                             interval - card.memory);
//         succ_prob = std::clamp(succ_prob, 0.0, 0.98);
//
//         // double fail_prob = (effective_difficulty < 0.5) ? fail_prob_easy : fail_prob_difficult;
//         double fail_prob = 1. - succ_prob;
//         std::bernoulli_distribution dist(fail_prob);
//         bool pass = !dist(gen); // Pass if not failing
//         card.review_count++;
//         if (card.id == 1) {
//             std::cout << "CardId: " << card.id << " pass: " << pass << " i: " << interval << " m: " << card.memory
//                       << "\n      prob: " << succ_prob << "\n";
//         }
//         if (pass) {
//             card.memory = std::min(interval * 2, card.memory * 2);
//         } else {
//             card.memory = 1;
//         }
//         // card.pass_history.push_back(pass);
//         return pass;
//     }
//
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
class SRS
{
public:
    virtual void schedule_card(Card& card, bool pass, double day) = 0;

    virtual void update_difficulty(Card& card, bool pass)
    {
        // Simple difficulty update based on pass/fail history
        // double pass_rate = card.pass_history.empty() ? 0.5 : std::accumulate(card.pass_history.begin(), card.pass_history.end(), 0.0) / card.pass_history.size();
        // card.srs_difficulty = 1.0 - pass_rate; // Higher difficulty for lower pass rate
    }

    virtual ~SRS() = default;
};

// FSRS4 implementation
class FSRS4 : public SRS
{
private:
    std::vector<double> weights; // [w0, w1, w2, w3] for stability calculation
public:
    FSRS4()
        : weights({0.4, 0.6, 2.0, 0.9}) {} // Initial weights

    void schedule_card(Card& card, bool pass, double day) override
    {
        update_difficulty(card, pass);
        double new_stability;
        if (card.is_new) {
            new_stability = weights[0] * (1.0 - card.srs_difficulty) + weights[1];
            card.is_new = false;
        } else {
            if (pass) {
                new_stability = card.stability * (weights[2] - card.srs_difficulty * weights[3]);
            } else {
                new_stability = card.stability / (weights[2] + card.srs_difficulty * weights[3]);
            }
        }
        card.stability = std::max(0.1, new_stability);
        card.interval = card.stability * (1.0 + 0.1 * card.review_count);
        card.next_review = day + card.interval;

        // Adapt weights based on pass/fail
        if (pass) {
            weights[0] += 0.001 * (1.0 - card.srs_difficulty);
            weights[2] += 0.001 * (1.0 - card.srs_difficulty);
        } else {
            weights[1] -= 0.001 * card.srs_difficulty;
            weights[3] += 0.001 * card.srs_difficulty;
        }
        // Ensure weights stay in reasonable bounds
        for (auto& w : weights)
            w = std::max(0.1, std::min(10.0, w));
    }
};

// FSRS5 implementation (simplified extension of FSRS4)
class FSRS5 : public SRS
{
private:
    std::vector<double> weights; // [w0, w1, w2, w3, w4] with memory decay
public:
    FSRS5()
        : weights({0.5, 0.7, 2.1, 0.8, 0.2}) {} // Initial weights

    void schedule_card(Card& card, bool pass, double day) override
    {
        update_difficulty(card, pass);
        double new_stability;
        if (card.is_new) {
            new_stability = weights[0] * (1.0 - card.srs_difficulty) + weights[1];
            card.is_new = false;
        } else {
            double memory_decay = std::exp(-weights[4] * card.interval);
            if (pass) {
                new_stability = card.stability * (weights[2] - card.srs_difficulty * weights[3]) * memory_decay;
            } else {
                new_stability = card.stability / (weights[2] + card.srs_difficulty * weights[3]) / memory_decay;
            }
        }
        card.stability = std::max(0.1, new_stability);
        card.interval = card.stability * (1.0 + 0.15 * card.review_count);
        card.next_review = day + card.interval;

        // Adapt weights
        if (pass) {
            weights[0] += 0.001 * (1.0 - card.srs_difficulty);
            weights[2] += 0.001 * (1.0 - card.srs_difficulty);
            weights[4] -= 0.0001 * card.srs_difficulty;
        } else {
            weights[1] -= 0.001 * card.srs_difficulty;
            weights[3] += 0.001 * card.srs_difficulty;
            weights[4] += 0.0001 * card.srs_difficulty;
        }
        for (auto& w : weights)
            w = std::max(0.1, std::min(10.0, w));
    }
};

// SuperMemo3 implementation
class SuperMemo3 : public SRS
{
public:
    void schedule_card(Card& card, bool pass, double day) override
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
    void schedule_card(Card& card, bool pass, double day) override
    {
        int nextInterval = scheduler.nextInterval(day, card.id, pass);
        card.next_review = day + nextInterval;
    }

    void clearItems() { scheduler.clearItems(); }

    void printWeights() { scheduler.printWeights(); }

private:
    Scheduler scheduler{};
};

// Simulation function with daily summary
void run_simulation(SRS* srs, const std::string& srs_name)
{
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
        for (auto& card : cards) {
            if (card.next_review <= day) {
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
        }

        if (cardIsPresent) {
            // Print daily summary
            double daily_pass_rate = daily_reviews > 0 ? (daily_passes / static_cast<double>(daily_reviews)) * 100 : 0.0;
            std::cout << "Day " << day << ": Reviews = " << daily_reviews
                      << ", Passes = " << daily_passes
                      << ", Pass Rate = " << daily_pass_rate << "%\n";
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
        auto srsData = SRS_data{};
        srsData.id = card.id;
        bool pass = user.review(card.id, day, false);
        // bool pass = user.review_card(card, day);
        if (pass) {
            retained_cards++;
        }
    }
    true_retention = static_cast<double>(retained_cards) / cards.size();

    std::cout << "\n"
              << srs_name << " Final Results:\n";
    std::cout << "Total Cards: " << cards.size() << "\n";
    std::cout << "Total Reviews: " << total_reviews << "\n";
    std::cout << "Successful Reviews: " << successful_reviews << "\n";
    std::cout << "Overall Pass Rate: " << (successful_reviews / total_reviews) * 100 << "%\n";
    std::cout << "Average True Retention: " << true_retention * 100 << "%\n";
    std::cout << "Retained Cards (>50% retention): " << retained_cards << "\n\n";
}

int main()
{
    FSRS4 fsrs4;
    FSRS5 fsrs5;
    SuperMemo3 sm3;
    MyScheduler scheduler;

    std::cout << "Running simulations for 365 days...\n\n";
    // run_simulation(&fsrs4, "FSRS4");
    // run_simulation(&fsrs5, "FSRS5");
    run_simulation(&scheduler, "Scheduler");
    scheduler.printWeights();
    scheduler.clearItems();
    run_simulation(&scheduler, "Scheduler");
    scheduler.printWeights();
    run_simulation(&sm3, "SuperMemo3");

    return 0;
}

//
//
//
//
//
//
//
//
// #include <cmath>
// #include <iostream>
// #include <random>
// #include <vector>
//
// struct TrainingData
// {
//     bool pass;
//     double previous_interval;
//     double difficulty;
//     int review_count;
//     double target_interval;
// };
//
// class NeuralNetwork
// {
// private:
//     std::vector<std::vector<double>> weights1; // Input to hidden
//     std::vector<std::vector<double>> weights2; // Hidden to output
//     std::vector<double> bias1;                 // Hidden layer bias
//     double bias2;                              // Output layer bias
//     const int input_size = 3;                  // previous_interval, card_difficulty, review_count
//     const int hidden_size = 10;
//     const int output_size = 1; // predicted_interval
//     const double learning_rate = 0.01;
//     const double target_pass_rate = 0.9;
//
//     // Activation functions
//     double sigmoid(double x) { return 1.0 / (1.0 + std::exp(-x)); }
//
//     double sigmoid_derivative(double x) { return x * (1.0 - x); }
//
//     // Normalize inputs
//     std::vector<double> normalize_input(double prev_interval, double difficulty, double review_count)
//     {
//         // Assuming typical ranges: prev_interval [1,100], difficulty [0,1], review_count [0,50]
//         return {
//                 prev_interval / 100.0,
//                 difficulty,
//                 review_count / 50.0};
//     }
//
//     // Denormalize output
//     double denormalize_output(double output, double prev_interval)
//     {
//         // Ensure output is at least prev_interval + 1
//         return std::max(prev_interval + 1.0, output * 100.0);
//     }
//
// public:
//     NeuralNetwork()
//     {
//         std::random_device rd;
//         std::mt19937 gen(rd());
//         std::normal_distribution<double> dist(0.0, 0.1);
//
//         // Initialize weights and biases
//         weights1.resize(input_size, std::vector<double>(hidden_size));
//         weights2.resize(hidden_size, std::vector<double>(output_size));
//         bias1.resize(hidden_size);
//
//         for (int i = 0; i < input_size; ++i)
//             for (int j = 0; j < hidden_size; ++j)
//                 weights1[i][j] = dist(gen);
//
//         for (int i = 0; i < hidden_size; ++i) {
//             bias1[i] = dist(gen);
//             for (int j = 0; j < output_size; ++j)
//                 weights2[i][j] = dist(gen);
//         }
//         bias2 = dist(gen);
//     }
//
//     double forward(const std::vector<double>& input)
//     {
//         std::vector<double> hidden(hidden_size);
//
//         // Input to hidden
//         for (int j = 0; j < hidden_size; ++j) {
//             hidden[j] = bias1[j];
//             for (int i = 0; i < input_size; ++i)
//                 hidden[j] += input[i] * weights1[i][j];
//             hidden[j] = sigmoid(hidden[j]);
//         }
//
//         // Hidden to output
//         double output = bias2;
//         for (int i = 0; i < hidden_size; ++i)
//             output += hidden[i] * weights2[i][0];
//
//         return output;
//     }
//
//     void train(const std::vector<TrainingData>& training_data, int epochs)
//     {
//         for (int epoch = 0; epoch < epochs; ++epoch) {
//             double total_error = 0.0;
//             for (const auto& data : training_data) {
//                 // Forward pass
//                 auto input = normalize_input(data.previous_interval, data.difficulty, data.review_count);
//                 std::vector<double> hidden(hidden_size);
//
//                 // Input to hidden
//                 for (int j = 0; j < hidden_size; ++j) {
//                     hidden[j] = bias1[j];
//                     for (int i = 0; i < input_size; ++i)
//                         hidden[j] += input[i] * weights1[i][j];
//                     hidden[j] = sigmoid(hidden[j]);
//                 }
//
//                 // Hidden to output
//                 double output = bias2;
//                 for (int i = 0; i < hidden_size; ++i)
//                     output += hidden[i] * weights2[i][0];
//
//                 // Calculate error (targeting 90% pass rate)
//                 double actual_interval = denormalize_output(output, data.previous_interval);
//                 double target = data.pass ? data.target_interval : data.target_interval * 0.7; // Reduce interval if failed
//                 double error = target - actual_interval;
//
//                 // Backpropagation
//                 double output_delta = error * learning_rate;
//
//                 // Update hidden to output weights
//                 for (int i = 0; i < hidden_size; ++i)
//                     weights2[i][0] += output_delta * hidden[i];
//                 bias2 += output_delta;
//
//                 // Update input to hidden weights
//                 std::vector<double> hidden_deltas(hidden_size);
//                 for (int j = 0; j < hidden_size; ++j) {
//                     hidden_deltas[j] = output_delta * weights2[j][0] * sigmoid_derivative(hidden[j]);
//                     for (int i = 0; i < input_size; ++i)
//                         weights1[i][j] += hidden_deltas[j] * input[i];
//                     bias1[j] += hidden_deltas[j];
//                 }
//
//                 total_error += std::abs(error);
//             }
//         }
//     }
//
//     double predict(double prev_interval, double difficulty, int review_count)
//     {
//         auto input = normalize_input(prev_interval, difficulty, review_count);
//         double output = forward(input);
//         return denormalize_output(output, prev_interval);
//     }
// };
//
// class CardSimulator
// {
// private:
//     struct Card
//     {
//         double previous_interval;
//         double difficulty;
//         int review_count;
//         bool active;
//     };
//
//     // struct TrainingData {
//     //     bool pass;
//     //     double previous_interval;
//     //     double difficulty;
//     //     int review_count;
//     //     double target_interval;
//     // };
//
//     std::vector<Card> cards;
//     NeuralNetwork nn;
//     std::mt19937 gen;
//     const int num_cards = 1000;
//     const double max_difficulty = 1.0;
//     const double difficulty_increment = 0.1;
//     const double max_interval = 200.0;
//     const double target_pass_rate = 0.9;
//
// public:
//     CardSimulator()
//         : gen(std::random_device{}())
//     {
//         // Initialize 1000 cards
//         cards.resize(num_cards);
//         for (auto& card : cards) {
//             card.previous_interval = 1.0;
//             card.difficulty = 0.0;
//             card.review_count = 0;
//             card.active = true;
//         }
//     }
//
//     // Calculate pass probability based on predicted interval
//     double calculate_pass_probability(double predicted_interval, double previous_interval, double difficulty)
//     {
//         double target_interval = previous_interval * (2.5 - difficulty);
//         double base_probability = target_pass_rate;
//
//         // Adjust probability: lower for longer intervals, higher for shorter
//         double interval_ratio = predicted_interval / target_interval;
//         // Logistic function to adjust probability between 0.7 and 0.95
//         double adjustment = 0.25 / (1.0 + std::exp(3.0 * (interval_ratio - 1.0)));
//         double probability = base_probability - adjustment + 0.125; // Centers around 0.9
//         return std::max(0.7, std::min(0.95, probability));
//     }
//
//     // Simulate one review cycle for all cards
//     std::vector<TrainingData> simulate_review_cycle()
//     {
//         std::vector<TrainingData> training_data;
//         std::bernoulli_distribution dist;
//
//         for (auto& card : cards) {
//             if (!card.active)
//                 continue;
//
//             // Predict interval
//             double predicted_interval = nn.predict(card.previous_interval, card.difficulty, card.review_count);
//
//             // Deactivate card if interval exceeds 200
//             if (predicted_interval > max_interval) {
//                 card.active = false;
//                 continue;
//             }
//
//             // Calculate pass probability
//             double pass_prob = calculate_pass_probability(predicted_interval, card.previous_interval, card.difficulty);
//             dist = std::bernoulli_distribution(pass_prob);
//
//             // Simulate review
//             bool pass = dist(gen);
//             card.review_count++;
//
//             // Update difficulty on fail
//             if (!pass) {
//                 card.difficulty = std::min(max_difficulty, card.difficulty + difficulty_increment);
//             }
//
//             // Store training data
//             TrainingData data;
//             data.pass = pass;
//             data.previous_interval = card.previous_interval;
//             data.difficulty = card.difficulty;
//             data.review_count = card.review_count;
//             data.target_interval = card.previous_interval * (2.5 - card.difficulty);
//             training_data.push_back(data);
//
//             // Update previous_interval
//             card.previous_interval = predicted_interval;
//         }
//
//         return training_data;
//     }
//
//     // Run simulation for multiple cycles
//     void run_simulation(int cycles, int epochs_per_cycle)
//     {
//         double total_pass_rate = 0.0;
//         int total_reviews = 0;
//         int active_cards = num_cards;
//
//         for (int cycle = 0; cycle < cycles; ++cycle) {
//             auto training_data = simulate_review_cycle();
//             if (training_data.empty())
//                 break;
//
//             // Train the neural network
//             nn.train(training_data, epochs_per_cycle);
//
//             // Calculate pass rate
//             int passes = 0;
//             for (const auto& data : training_data) {
//                 if (data.pass)
//                     passes++;
//             }
//             double pass_rate = static_cast<double>(passes) / training_data.size();
//             total_pass_rate += pass_rate;
//             total_reviews += training_data.size();
//
//             // Count active cards
//             active_cards = 0;
//             for (const auto& card : cards) {
//                 if (card.active)
//                     active_cards++;
//             }
//
//             std::cout << "Cycle " << cycle + 1 << ": Pass rate = " << pass_rate * 100.0
//                       << "%, Active cards = " << active_cards << "\n";
//         }
//
//         double avg_pass_rate = total_pass_rate / cycles;
//         std::cout << "Average pass rate: " << avg_pass_rate * 100.0 << "%\n";
//         std::cout << "Total reviews: " << total_reviews << "\n";
//         std::cout << "Final active cards: " << active_cards << "\n";
//     }
// };
//
// int main()
// {
//     CardSimulator simulator;
//     simulator.run_simulation(100, 100); // 100 cycles, 100 epochs per cycle
//     return 0;
// }
