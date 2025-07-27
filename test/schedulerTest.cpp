#include <cmath>
#include <ctime>
#include <iostream>
#include <random>
#include <vector>

// Struct to represent a flashcard item
struct Item
{
    double stability;        // Memory stability (days)
    double difficulty;       // Item difficulty (0 to 1, higher is harder)
    double last_review_time; // Time of last review (days since start)
    int last_rating;         // Last rating given (1=Again, 2=Hard, 3=Good, 4=Easy)
    int review_count;        // Number of reviews
};

// FSRS v5.5 Scheduler class with 17 parameters
class FSRSScheduler
{
private:
    double desired_retention = 0.9; // Target retention at review time
    std::vector<double> weights;    // 17 parameters
    double learning_rate = 0.01;    // For training weights
    std::vector<Item> items;

    // Initialize 17 weights:
    // w[0-3]: Initial stability for ratings 1-4
    // w[4-7]: Stability increase factors for ratings 1-4
    // w[8-11]: Stability decrease factors for ratings 1-4
    // w[12]: Base decay rate (D)
    // w[13]: Difficulty scaling factor
    // w[14-16]: Additional factors (e.g., retrievability, user memory, context)
    void initialize_weights()
    {
        weights = {
                0.4, 0.6, 1.0, 1.2, // w[0-3]: Initial stabilities
                0.4, 0.9, 1.5, 2.5, // w[4-7]: Stability increase factors
                0.3, 0.8, 1.0, 1.0, // w[8-11]: Stability decrease factors
                1.0,                // w[12]: Decay rate
                0.5,                // w[13]: Difficulty scaling
                1.0, 1.0, 1.0       // w[14-16]: Additional factors
        };

        // weights = {
        //         0.1873, 0.7846, 6.7259, 28.1730, 
        //         5.3135, 1.3815, 0.8716, 0.0180, 
        //         1.4517, 0.1072, 0.8504, 2.1134,
        //         0.0779, 
        //         0.3982, 
        //         1.6768, 0.3682, 2.8755};
    }

    // Retention function: R(t) = (1 + t / (9 * S * w[14]))^(-w[12])
    double retention(double stability, double time) const
    {
        return std::pow(1.0 + time / (9.0 * stability * weights[14]), -weights[12]);
    }

    // Calculate next review interval
    double next_interval(double stability) const
    {
        return 9.0 * stability * weights[14] * (std::pow(1.0 / desired_retention, 1.0 / weights[12]) - 1.0);
    }

    // Update stability based on rating
    void update_stability(Item& item, int rating)
    {
        double factor = (rating > 1) ? weights[4 + rating - 1] : weights[8]; // Increase for success, decrease for failure
        item.stability *= factor;
        // Adjust stability based on difficulty and w[13]
        item.stability *= (1.0 - weights[13] * item.difficulty);
        // Apply additional factor w[15] (e.g., user memory strength)
        item.stability *= weights[15];
        // Ensure stability doesn't go below a minimum
        item.stability = std::max(item.stability, 0.1);
        item.last_rating = rating;
        item.review_count++;
    }

    // Train weights based on review outcome
    void train_weights(Item& item, int rating, double time_since_review)
    {
        // Infer actual retention: 1 for success (Hard, Good, Easy), 0 for failure (Again)
        double actual_retention = (rating > 1) ? 1.0 : 0.0;
        double predicted_retention = retention(item.stability, time_since_review);
        double error = actual_retention - predicted_retention;

        // Gradient for decay rate (w[12])
        double t_over_9s = time_since_review / (9.0 * item.stability * weights[14]);
        weights[12] += learning_rate * error * predicted_retention * std::log(1.0 + t_over_9s);
        weights[12] = std::max(0.5, std::min(weights[12], 2.0));

        // Gradient for retrievability factor (w[14])
        weights[14] += learning_rate * error * predicted_retention * (weights[12] * time_since_review) / (9.0 * item.stability * weights[14] * (1.0 + t_over_9s));
        weights[14] = std::max(0.5, std::min(weights[14], 2.0));

        // Update stability-related weights based on rating
        if (item.review_count == 1) {
            // Update initial stability weight for the given rating
            weights[rating - 1] += learning_rate * error * (1.0 - weights[13] * item.difficulty);
            weights[rating - 1] = std::max(0.1, weights[rating - 1]);
        } else {
            // Update increase/decrease factor
            int weight_index = (rating > 1) ? (4 + rating - 1) : 8;
            weights[weight_index] += learning_rate * error;
            weights[weight_index] = std::max(0.1, std::min(weights[weight_index], 3.0));
        }

        // Update difficulty scaling (w[13])
        weights[13] += learning_rate * error * item.difficulty;
        weights[13] = std::max(0.0, std::min(weights[13], 1.0));

        // Update user memory factor (w[15])
        weights[15] += learning_rate * error;
        weights[15] = std::max(0.5, std::min(weights[15], 2.0));

        // Placeholder for w[16] (context factor, minimally updated)
        weights[16] += learning_rate * error * 0.1;
        weights[16] = std::max(0.5, std::min(weights[16], 2.0));
    }

public:
    FSRSScheduler()
    {
        initialize_weights();
    }

    // Add a new item
    void add_item(double difficulty)
    {
        Item item;
        item.stability = weights[3] * (1.0 - weights[13] * difficulty); // Use w[3] (Easy) as default
        item.difficulty = difficulty;
        item.last_review_time = 0.0;
        item.last_rating = 0;
        item.review_count = 0;
        items.push_back(item);
    }

    // Simulate a review for an item
    void review_item(int item_index, int rating, double current_time)
    {
        Item& item = items[item_index];
        double time_since_review = current_time - item.last_review_time;

        // Train weights
        train_weights(item, rating, time_since_review);

        // Update stability and last review time
        update_stability(item, rating);
        item.last_review_time = current_time;

        // Output review information
        std::cout << "Item " << item_index << ": Rating = " << rating
                  << ", Stability = " << item.stability
                  << ", Next review in " << next_interval(item.stability) << " days\n";
    }

    // Get next item to review
    int get_next_item(double current_time) const
    {
        for (size_t i = 0; i < items.size(); ++i) {
            double time_since_review = current_time - items[i].last_review_time;
            if (retention(items[i].stability, time_since_review) <= desired_retention) {
                return i;
            }
        }
        return -1; // No item due for review
    }

    // Print current weights
    void print_weights() const
    {
        std::cout << "Current Weights:\n";
        for (size_t i = 0; i < weights.size(); ++i) {
            std::cout << "w[" << i << "] = " << weights[i] << "\n";
        }
    }
};

// Main function to simulate FSRS scheduling
int main()
{
    FSRSScheduler scheduler;

    // Add three items with different difficulties
    scheduler.add_item(0.0); // Easy item
    scheduler.add_item(0.5); // Medium item
    // scheduler.add_item(0.8); // Hard item

    // Simulate 30 days of reviews
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rating_dist(1, 4);

    for (int day = 1; day <= 365; ++day) {
        int item_index = scheduler.get_next_item(day);
        bool logged = false;
        while (item_index != -1) {
            if (!logged) {
                std::cout << "\nDay " << day << ":\n";
                logged = true;
            }
            // Simulate user rating (random for demo)
            int rating = 3; // rating_dist(gen);
            std::cout << "Reviewing item " << item_index << " with rating " << rating << "\n";
            scheduler.review_item(item_index, rating, day);
            item_index = scheduler.get_next_item(day);
        }
    }

    // Output final weights
    scheduler.print_weights();

    return 0;
}

// #include <iostream>
// #include <vector>
// #include <cmath>
// #include <random>
// #include <ctime>

// #include <vector>
// #include <cmath>
// #include <random>
// #include <ctime>
//
// class NeuralNetwork {
// private:
//     std::vector<std::vector<double>> weights_hidden; // Weights from input to hidden layer
//     std::vector<double> weights_output; // Weights from hidden to output layer
//     std::vector<double> bias_hidden;
//     double bias_output;
//     int input_size = 3;
//     int hidden_size = 10;
//     double learning_rate = 0.01;
//
//     // Sigmoid activation function
//     double sigmoid(double x) {
//         return 1.0 / (1.0 + std::exp(-x));
//     }
//
//     // Derivative of sigmoid
//     double sigmoid_derivative(double x) {
//         double s = sigmoid(x);
//         return s * (1.0 - s);
//     }
//
// public:
//     NeuralNetwork() {
//         // Initialize random number generator
//         std::mt19937 gen(static_cast<unsigned>(time(0)));
//         std::normal_distribution<double> dist(0.0, 1.0);
//
//         // Initialize weights and biases
//         weights_hidden.resize(input_size, std::vector<double>(hidden_size));
//         weights_output.resize(hidden_size);
//         bias_hidden.resize(hidden_size);
//
//         // Initialize weights with random values
//         for (int i = 0; i < input_size; ++i) {
//             for (int j = 0; j < hidden_size; ++j) {
//                 weights_hidden[i][j] = dist(gen) * 0.1;
//             }
//         }
//         for (int j = 0; j < hidden_size; ++j) {
//             weights_output[j] = dist(gen) * 0.1;
//             bias_hidden[j] = dist(gen) * 0.1;
//         }
//         bias_output = dist(gen) * 0.1;
//     }
//
//     // Forward pass
//     double forward(const std::vector<double>& inputs) {
//         std::vector<double> hidden_activations(hidden_size);
//
//         // Calculate hidden layer activations
//         for (int j = 0; j < hidden_size; ++j) {
//             double sum = bias_hidden[j];
//             for (int i = 0; i < input_size; ++i) {
//                 sum += inputs[i] * weights_hidden[i][j];
//             }
//             hidden_activations[j] = sigmoid(sum);
//         }
//
//         // Calculate output
//         double output_sum = bias_output;
//         for (int j = 0; j < hidden_size; ++j) {
//             output_sum += hidden_activations[j] * weights_output[j];
//         }
//         return sigmoid(output_sum);
//     }
//
//     // Train the network
//     void train(const std::vector<std::vector<double>>& inputs,
//               const std::vector<double>& targets, int epochs) {
//         for (int epoch = 0; epoch < epochs; ++epoch) {
//             double total_error = 0.0;
//             for (size_t i = 0; i < inputs.size(); ++i) {
//                 // Forward pass
//                 std::vector<double> hidden_activations(hidden_size);
//                 std::vector<double> hidden_sums(hidden_size);
//
//                 // Calculate hidden layer
//                 for (int j = 0; j < hidden_size; ++j) {
//                     double sum = bias_hidden[j];
//                     for (int k = 0; k < input_size; ++k) {
//                         sum += inputs[i][k] * weights_hidden[k][j];
//                     }
//                     hidden_sums[j] = sum;
//                     hidden_activations[j] = sigmoid(sum);
//                 }
//
//                 // Calculate output
//                 double output_sum = bias_output;
//                 for (int j = 0; j < hidden_size; ++j) {
//                     output_sum += hidden_activations[j] * weights_output[j];
//                 }
//                 double output = sigmoid(output_sum);
//
//                 // Calculate error
//                 double error = targets[i] - output;
//                 total_error += error * error;
//
//                 // Backpropagation
//                 double output_delta = error * sigmoid_derivative(output_sum);
//
//                 // Update output weights and bias
//                 for (int j = 0; j < hidden_size; ++j) {
//                     weights_output[j] += learning_rate * output_delta * hidden_activations[j];
//                 }
//                 bias_output += learning_rate * output_delta;
//
//                 // Update hidden layer weights and biases
//                 std::vector<double> hidden_deltas(hidden_size);
//                 for (int j = 0; j < hidden_size; ++j) {
//                     hidden_deltas[j] = output_delta * weights_output[j] * sigmoid_derivative(hidden_sums[j]);
//                     for (int k = 0; k < input_size; ++k) {
//                         weights_hidden[k][j] += learning_rate * hidden_deltas[j] * inputs[i][k];
//                     }
//                     bias_hidden[j] += learning_rate * hidden_deltas[j];
//                 }
//             }
//         }
//     }
// };
//
// // Function to generate random training data
// std::vector<std::vector<double>> generate_training_data(int samples) {
//     std::mt19937 gen(static_cast<unsigned>(time(0)));
//     std::uniform_real_distribution<double> dist_a_b(3.0, 5.0);
//     std::uniform_real_distribution<double> dist_c(0.0, 1.0);
//     std::uniform_real_distribution<double> dist_prob(0.0, 1.0);
//
//     std::vector<std::vector<double>> data(samples, std::vector<double>(3));
//     std::vector<double> targets(samples);
//
//     for (int i = 0; i < samples; ++i) {
//         double a = dist_a_b(gen);
//         double b = dist_a_b(gen);
//         double c = dist_c(gen);
//         double prob = ((a + b) * c) / 10.0;
//         data[i] = {a, b, c};
//         targets[i] = (dist_prob(gen) < prob) ? 1.0 : 0.0;
//     }
//
//     return data;
// }
//
// std::vector<double> generate_targets(const std::vector<std::vector<double>>& data) {
//     std::mt19937 gen(static_cast<unsigned>(time(0)));
//     std::uniform_real_distribution<double> dist_prob(0.0, 1.0);
//     std::vector<double> targets(data.size());
//
//     for (size_t i = 0; i < data.size(); ++i) {
//         double prob = ((data[i][0] + data[i][1]) * data[i][2]) / 10.0;
//         targets[i] = (dist_prob(gen) < prob) ? 1.0 : 0.0;
//     }
//
//     return targets;
// }
//
// int main() {
//     NeuralNetwork nn;
//
//     // Generate training data
//     int training_samples = 1000;
//     auto training_data = generate_training_data(training_samples);
//     auto targets = generate_targets(training_data);
//
//     // Train the network
//     nn.train(training_data, targets, 1000);
//
//     // Generate test data
//     int test_samples = 5;
//     std::mt19937 gen(static_cast<unsigned>(time(0)));
//     std::uniform_real_distribution<double> dist_a_b(3.0, 5.0);
//     std::uniform_real_distribution<double> dist_c(0.0, 1.0);
//
//     std::cout << "\nTest Results:\n";
//     std::cout << "a\tb\tc\tCalculated Prob\tPredicted Prob\tDifference\n";
//     std::cout << "------------------------------------------------------------\n";
//
//     for (int i = 0; i < test_samples; ++i) {
//         std::vector<double> test_input = {dist_a_b(gen), dist_a_b(gen), dist_c(gen)};
//         double calculated_prob = ((test_input[0] + test_input[1]) * test_input[2]) / 10.0;
//         double predicted_prob = nn.forward(test_input);
//
//         std::cout << test_input[0] << "\t"
//                   << test_input[1] << "\t"
//                   << test_input[2] << "\t"
//                   << calculated_prob << "\t\t"
//                   << predicted_prob << "\t\t"
//                   << std::abs(calculated_prob - predicted_prob) << "\n";
//     }
//
//     return 0;
// }
// -----------------------------------------------------------------------------------------------------------
// #include <vector>
// #include <random>
// #include <cmath>
// #include <algorithm>
// #include <numeric>
//
// // Random number generator setup
// std::random_device rd;
// std::mt19937 gen(rd());
//
// // Card structure
// struct Card {
//     int id;
//     double true_difficulty; // Ground truth difficulty (0 to 1)
//     int review_count;
//     bool is_new;
//     double srs_difficulty; // SRS-estimated difficulty
//     double stability; // For FSRS
//     double interval; // Current interval in days
//     double next_review; // Day for next review
//     std::vector<bool> pass_history; // true for pass, false for fail
// };
//
// // User model
// class User {
// private:
//     double fail_prob_easy; // Base fail probability for easy cards
//     double fail_prob_difficult; // Base fail probability for difficult cards
//     double difficulty_reduction; // Reduction in difficulty per review
// public:
//     User() : fail_prob_easy(0.1), fail_prob_difficult(0.7), difficulty_reduction(0.05) {}
//
//     bool review_card(Card& card, double day) {
//         double effective_difficulty = card.true_difficulty * (1.0 - card.review_count * difficulty_reduction);
//         effective_difficulty = std::max(0.0, std::min(1.0, effective_difficulty));
//         double fail_prob = (effective_difficulty < 0.5) ? fail_prob_easy : fail_prob_difficult;
//         std::bernoulli_distribution dist(fail_prob);
//         bool pass = !dist(gen); // Pass if not failing
//         card.review_count++;
//         card.pass_history.push_back(pass);
//         return pass;
//     }
//
//     Card create_card(int id, double day) {
//         Card card;
//         card.id = id;
//         std::uniform_real_distribution<> dist(0.0, 1.0);
//         card.true_difficulty = dist(gen); // Random difficulty between 0 and 1
//         card.review_count = 0;
//         card.is_new = true;
//         card.srs_difficulty = 0.5; // Initial estimate
//         card.stability = 2.0; // Initial stability for FSRS
//         card.interval = 1.0;
//         card.next_review = day;
//         return card;
//     }
// };
//
// // Base SRS class
// class SRS {
// public:
//     virtual void schedule_card(Card& card, bool pass, double day) = 0;
//     virtual void update_difficulty(Card& card, bool pass) {
//         // Simple difficulty update based on pass/fail history
//         double pass_rate = card.pass_history.empty() ? 0.5 :
//             std::accumulate(card.pass_history.begin(), card.pass_history.end(), 0.0) / card.pass_history.size();
//         card.srs_difficulty = 1.0 - pass_rate; // Higher difficulty for lower pass rate
//     }
//     virtual ~SRS() = default;
// };
//
// // FSRS4 implementation
// class FSRS4 : public SRS {
// private:
//     std::vector<double> weights; // [w0, w1, w2, w3] for stability calculation
// public:
//     FSRS4() : weights({0.4, 0.6, 2.0, 0.9}) {} // Initial weights
//
//     void schedule_card(Card& card, bool pass, double day) override {
//         update_difficulty(card, pass);
//         double new_stability;
//         if (card.is_new) {
//             new_stability = weights[0] * (1.0 - card.srs_difficulty) + weights[1];
//             card.is_new = false;
//         } else {
//             if (pass) {
//                 new_stability = card.stability * (weights[2] - card.srs_difficulty * weights[3]);
//             } else {
//                 new_stability = card.stability / (weights[2] + card.srs_difficulty * weights[3]);
//             }
//         }
//         card.stability = std::max(0.1, new_stability);
//         card.interval = card.stability * (1.0 + 0.1 * card.review_count);
//         card.next_review = day + card.interval;
//
//         // Adapt weights based on pass/fail
//         if (pass) {
//             weights[0] += 0.001 * (1.0 - card.srs_difficulty);
//             weights[2] += 0.001 * (1.0 - card.srs_difficulty);
//         } else {
//             weights[1] -= 0.001 * card.srs_difficulty;
//             weights[3] += 0.001 * card.srs_difficulty;
//         }
//         // Ensure weights stay in reasonable bounds
//         for (auto& w : weights) w = std::max(0.1, std::min(10.0, w));
//     }
// };
//
// // FSRS5 implementation (simplified extension of FSRS4)
// class FSRS5 : public SRS {
// private:
//     std::vector<double> weights; // [w0, w1, w2, w3, w4] with memory decay
// public:
//     FSRS5() : weights({0.5, 0.7, 2.1, 0.8, 0.2}) {} // Initial weights
//
//     void schedule_card(Card& card, bool pass, double day) override {
//         update_difficulty(card, pass);
//         double new_stability;
//         if (card.is_new) {
//             new_stability = weights[0] * (1.0 - card.srs_difficulty) + weights[1];
//             card.is_new = false;
//         } else {
//             double memory_decay = std::exp(-weights[4] * card.interval);
//             if (pass) {
//                 new_stability = card.stability * (weights[2] - card.srs_difficulty * weights[3]) * memory_decay;
//             } else {
//                 new_stability = card.stability / (weights[2] + card.srs_difficulty * weights[3]) / memory_decay;
//             }
//         }
//         card.stability = std::max(0.1, new_stability);
//         card.interval = card.stability * (1.0 + 0.15 * card.review_count);
//         card.next_review = day + card.interval;
//
//         // Adapt weights
//         if (pass) {
//             weights[0] += 0.001 * (1.0 - card.srs_difficulty);
//             weights[2] += 0.001 * (1.0 - card.srs_difficulty);
//             weights[4] -= 0.0001 * card.srs_difficulty;
//         } else {
//             weights[1] -= 0.001 * card.srs_difficulty;
//             weights[3] += 0.001 * card.srs_difficulty;
//             weights[4] += 0.0001 * card.srs_difficulty;
//         }
//         for (auto& w : weights) w = std::max(0.1, std::min(10.0, w));
//     }
// };
//
// // SuperMemo3 implementation
// class SuperMemo3 : public SRS {
// public:
//     void schedule_card(Card& card, bool pass, double day) override {
//         update_difficulty(card, pass);
//         double easiness = 2.5 - card.srs_difficulty; // Easiness factor
//         easiness = std::max(1.3, easiness);
//         if (card.is_new) {
//             card.interval = 1.0;
//             card.is_new = false;
//         } else {
//             if (pass) {
//                 if (card.review_count == 1) card.interval = 1.0;
//                 else if (card.review_count == 2) card.interval = 6.0;
//                 else card.interval *= easiness;
//             } else {
//                 card.interval = 1.0;
//                 easiness = std::max(1.3, easiness - 0.3);
//             }
//         }
//         card.next_review = day + card.interval;
//         card.srs_difficulty = 2.5 - easiness; // Update difficulty based on easiness
//     }
// };
//
// // Simulation function with daily summary
// void run_simulation(SRS* srs, const std::string& srs_name) {
//     User user;
//     std::vector<Card> cards;
//     int card_id = 0;
//     double total_reviews = 0;
//     double successful_reviews = 0;
//
//     std::cout << srs_name << " Daily Summaries:\n";
//     std::cout << "-----------------------------------\n";
//
//     for (int day = 1; day <= 365; ++day) {
//         // Add 20 new cards
//         for (int i = 0; i < 20; ++i) {
//             cards.push_back(user.create_card(card_id++, day));
//         }
//
//         // Review due cards
//         int daily_reviews = 0;
//         int daily_passes = 0;
//         for (auto& card : cards) {
//             if (card.next_review <= day) {
//                 bool pass = user.review_card(card, day);
//                 srs->schedule_card(card, pass, day);
//                 daily_reviews++;
//                 total_reviews++;
//                 if (pass) {
//                     daily_passes++;
//                     successful_reviews++;
//                 }
//             }
//         }
//
//         // Print daily summary
//         double daily_pass_rate = daily_reviews > 0 ? (daily_passes / static_cast<double>(daily_reviews)) * 100 : 0.0;
//         std::cout << "Day " << day << ": Reviews = " << daily_reviews
//                   << ", Passes = " << daily_passes
//                   << ", Pass Rate = " << daily_pass_rate << "%\n";
//     }
//
//     // Calculate true retention
//     double true_retention = 0.0;
//     int retained_cards = 0;
//     for (const auto& card : cards) {
//         double effective_difficulty = card.true_difficulty * (1.0 - card.review_count * 0.05);
//         effective_difficulty = std::max(0.0, std::min(1.0, effective_difficulty));
//         double retention_prob = 1.0 - (effective_difficulty < 0.5 ? 0.1 : 0.7);
//         true_retention += retention_prob;
//         if (retention_prob > 0.5) retained_cards++;
//     }
//     true_retention /= cards.size();
//
//     std::cout << "\n" << srs_name << " Final Results:\n";
//     std::cout << "Total Cards: " << cards.size() << "\n";
//     std::cout << "Total Reviews: " << total_reviews << "\n";
//     std::cout << "Successful Reviews: " << successful_reviews << "\n";
//     std::cout << "Overall Pass Rate: " << (successful_reviews / total_reviews) * 100 << "%\n";
//     std::cout << "Average True Retention: " << true_retention * 100 << "%\n";
//     std::cout << "Retained Cards (>50% retention): " << retained_cards << "\n\n";
// }
//
// int main() {
//     FSRS4 fsrs4;
//     FSRS5 fsrs5;
//     SuperMemo3 sm3;
//
//     std::cout << "Running simulations for 365 days...\n\n";
//     run_simulation(&fsrs4, "FSRS4");
//     run_simulation(&fsrs5, "FSRS5");
//     run_simulation(&sm3, "SuperMemo3");
//
//     return 0;
// }
// ----------------------------------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------------------------------------------------------------------------

// #include <database/SpacedRepetitionData.h>

// #include <database/TokenizationChoiceDB.h>
// #include <database/TokenizationChoiceDbChi.h>
// #include <database/WordDB.h>
// #include <dictionary/Dictionary.h>
// #include <dictionary/DictionaryChi.h>
// #include <dictionary/DictionaryJpn.h>
// #include <misc/Config.h>
// #include <misc/Identifier.h>
// #include <misc/Language.h>
// #include <spaced_repetition/DataBase.h>
// #include <spaced_repetition/ITreeWalker.h>
// #include <spaced_repetition/Scheduler.h>
// #include <spaced_repetition/Scheduler2.h>
// #include <spdlog/common.h>
// #include <spdlog/spdlog.h>
// #include <utils/StringU8.h>
// #include <utils/format.h>
//
// #include <algorithm>
// #include <array>
// #include <chrono>
// #include <cstddef>
// #include <cstdlib>
// #include <filesystem>
// #include <memory>
// #include <ratio>
// #include <string>
// #include <utility>
//
// namespace fs = std::filesystem;
//
// void genMatrix()
// {
//     std::array<int, 16> values{};
//     std::array<double, 16> factors{};
//     for (double fac : {1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3.0}) {
//         {
//             double val = 1.;
//             double lval = 0;
//             for (auto& v : values) {
//                 v = static_cast<int>(val);
//                 if (lval != 0) {
//                     if (v - 1 != lval) {
//                         if (std::abs(static_cast<double>(v) - (lval * fac)) > std::abs(static_cast<double>(v - 1) - (lval * fac))) {
//                             v--;
//                         }
//                     }
//                     if (std::abs(static_cast<double>(v) - (lval * fac)) > std::abs(static_cast<double>(v + 1) - (lval * fac))) {
//                         v++;
//                     }
//                 }
//                 lval = static_cast<double>(v);
//                 val *= fac;
//                 val = std::min(4242., val);
//             }
//             fmt::print("{{{:4d}}},\n", fmt::join(values, "}, {"));
//         }
//         {
//             double val = 1;
//             double lval = 1;
//             std::size_t i = 0;
//             for (auto& f : factors) {
//                 val = values.at(i++);
//                 f = val / lval;
//                 lval = val;
//             }
//             fmt::print("{{{:.2f}}},\n", fmt::join(factors, "}, {"));
//         }
//     }
//     // {
//     //     double val = 1;
//     //     double lval = 1;
//     //     for (auto& v : values) {
//     //         v = static_cast<int>(val);
//     //         lval = std::exchange(val, val + lval);
//     //         val = std::min(4242., val);
//     //     }
//     //     fmt::print("{{{:4d}}},\n", fmt::join(values, "}, {"));
//     // }
//
//     // {
//     //     double val = 1;
//     //     double lval = 1;
//     //     std::size_t i = 0;
//     //     for (auto& f : factors) {
//     //         val = values.at(i++);
//     //         f = val / lval;
//     //         lval = val;
//     //     }
//     //     fmt::print("{{{:.2f}}},\n", fmt::join(factors, "}, {"));
//     // }
//     // for (double fac : {1., 2., 3.} /* {0.5, 0.7, 0.85, 1.0, 2.} */) {
//     //     {
//     //         double val = 1;
//     //         double lval = 1;
//     //         for (auto& v : values) {
//     //             v = static_cast<int>(val);
//     //             lval = std::exchange(val, val + (lval * fac));
//     //             val = std::min(4242., val);
//     //         }
//     //         fmt::print("{{{:4d}}},\n", fmt::join(values, "}, {"));
//     //     }
//     //     {
//     //         double val = 1;
//     //         double lval = 1;
//     //         std::size_t i = 0;
//     //         for (auto& f : factors) {
//     //             val = values.at(i++);
//     //             f = val / lval;
//     //             lval = val;
//     //         }
//     //         fmt::print("{{{:.2f}}},\n", fmt::join(factors, "}, {"));
//     //     }
//     // }
// }
//
// void printValues()
// {
//     fmt::print("\n");
//     for (unsigned lastInterval = 0; lastInterval <= 62; lastInterval++) {
//         fmt::print("{:4d}: ", lastInterval);
//         for (int skew = -8; skew <= 10; skew++) {
//             unsigned interval = sr::Scheduler2::nextIntervalDays(lastInterval, skew);
//             if (interval == 0) {
//                 if (skew < 0) {
//                     fmt::print("[       ], ");
//                 } else {
//                     fmt::print("[      ], ");
//                 }
//             } else {
//                 fmt::print("[{}:{:4d}], ", skew, interval);
//             }
//         }
//         fmt::print("\n");
//     }
// }
//
// auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
// {
//     auto path_to_exe = fs::read_symlink("/proc/self/exe");
//     return std::make_shared<zikhron::Config>(path_to_exe.parent_path());
// }
//
// auto main() -> int
// {
//     printValues();
//     return 0;
//     using Rating = sr::Rating;
//     auto scheduler = sr::Scheduler{};
//     auto review = [&scheduler](const database::SpacedRepetitionData& srd, Rating rating) -> database::SpacedRepetitionData {
//         database::SpacedRepetitionData srdTemp = srd;
//         using Days = std::chrono::duration<double, std::ratio<86400>>;
//         // auto dur = scheduler.getIntervalDays(srd);
//         auto dur = srd.due - srd.reviewed;
//         srdTemp.reviewed = srd.reviewed - dur; //+ duration_cast<std::chrono::nanoseconds>(Days{srd.shiftBackward});
//         srdTemp.due = std::chrono::system_clock::now();
//
//         auto srsFail = scheduler.review(srdTemp, Rating::fail);
//         auto srsPass = scheduler.review(srdTemp, Rating::pass);
//         // auto srsEasy = scheduler.review(srd, Rating::familiar);
//         spdlog::info("incd: {}, ----> {}", srdTemp.getDueInTimeLabel(), srdTemp.serialize());
//         spdlog::info("fail: {}, ----> {}", srsFail.getDueInTimeLabel(), srsFail.serialize());
//         spdlog::info("pass: {}, ----> {}", srsPass.getDueInTimeLabel(), srsPass.serialize());
//         // spdlog::info("easy: {}", srsEasy.getDueInTimeLabel());
//         switch (rating) {
//         case Rating::fail:
//             spdlog::info("Fail");
//             return srsFail;
//         case Rating::pass:
//             spdlog::info("Pass");
//             return srsPass;
//         case Rating::familiar:
//             spdlog::info("Familiar");
//             return srsPass;
//         }
//         std::unreachable();
//     };
//     // std::string ser = "2025-02-27 19:28:08,2025-03-02 01:00:00,0,0,1.0000,1.5850,review,true,1018,0,1,";
//     std::string ser = "2025-03-11 07:39:36,2025-03-11 07:49:36,200,200,1.2000,0.9670,relearning,true,683,653,0,1,";
//     // database::SpacedRepetitionData srd{};
//     auto srd = database::SpacedRepetitionData::deserialize(ser);
//     srd = review(srd, Rating::fail);
//     srd = review(srd, Rating::pass);
//     srd = review(srd, Rating::pass);
//     srd = review(srd, Rating::fail);
//     srd = review(srd, Rating::pass);
//     srd = review(srd, Rating::pass);
//     srd = review(srd, Rating::fail);
//     srd = review(srd, Rating::pass);
//     srd = review(srd, Rating::pass);
//     srd = review(srd, Rating::fail);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::fail);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::fail);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::fail);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::fail);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::fail);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::fail);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::fail);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//     // srd = review(srd, Rating::pass);
//
//     spdlog::info("Time: {}", srd.getDueInTimeLabel());
// }
