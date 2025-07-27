#pragma once
#include <cmath>
#include <ctime>
#include <random>
#include <vector>

class LSTM
{
public:
    LSTM();

    std::vector<double> Wout; // Output layer weights
    double bout;              // Output layer bias

    // Forward pass for a single time step
    void forward_step(const std::vector<double>& x,
                      std::vector<double>& h_prev,
                      std::vector<double>& c_prev,
                      std::vector<double>& h,
                      std::vector<double>& c,
                      std::vector<double>& f,
                      std::vector<double>& i,
                      std::vector<double>& g,
                      std::vector<double>& o);

    // Forward pass for the entire sequence
    auto forward(const std::vector<double>& inputs) -> double;

    // Train the LSTM
    void train(const std::vector<std::vector<double>>& inputs,
               const std::vector<double>& targets,
               int epochs);

private:
    // Weights for input, forget, cell, and output gates
    std::vector<std::vector<double>> Wf, Wi, Wc, Wo; // Input weights
    std::vector<std::vector<double>> Uf, Ui, Uc, Uo; // Recurrent weights
    std::vector<double> bf, bi, bc, bo;              // Biases
    int input_size = 3;
    static constexpr std::size_t hidden_size = 10;
    double learning_rate = 0.01;

    // Sigmoid and tanh activation functions
    static auto sigmoid(double x) -> double
    {
        return 1.0 / (1.0 + std::exp(-x));
    }

    static auto tanh(double x) -> double
    {
        return std::tanh(x);
    }

    static auto sigmoid_derivative(double x) -> double
    {
        double s = sigmoid(x);
        return s * (1.0 - s);
    }

    static auto tanh_derivative(double x) -> double
    {
        double t = tanh(x);
        return 1.0 - (t * t);
    }
};


// Function to generate random training data
inline auto generate_training_data(int samples) -> std::vector<std::vector<double>>
{
    std::mt19937 gen(static_cast<unsigned>(time(0)));
    std::uniform_real_distribution<double> dist_a_b(3.0, 5.0);
    std::uniform_real_distribution<double> dist_c(0.0, 1.0);
    std::uniform_real_distribution<double> dist_prob(0.0, 1.0);

    std::vector<std::vector<double>> data(samples, std::vector<double>(3));
    std::vector<double> targets(samples);

    for (int i = 0; i < samples; ++i) {
        double a = dist_a_b(gen);
        double b = dist_a_b(gen);
        double c = dist_c(gen);
        double prob = ((a + b) * c) / 10.0;
        data[i] = {a, b, c};
        targets[i] = (dist_prob(gen) < prob) ? 1.0 : 0.0;
    }

    return data;
}

inline auto generate_targets(const std::vector<std::vector<double>>& data) -> std::vector<double>
{
    std::mt19937 gen(static_cast<unsigned>(time(0)));
    std::uniform_real_distribution<double> dist_prob(0.0, 1.0);
    std::vector<double> targets(data.size());

    for (size_t i = 0; i < data.size(); ++i) {
        double prob = ((data[i][0] + data[i][1]) * data[i][2]) / 10.0;
        targets[i] = (dist_prob(gen) < prob) ? 1.0 : 0.0;
    }

    return targets;
}


// int main()
// {
//     LSTM lstm;
//
//     // Generate training data
//     int training_samples = 1000;
//     auto training_data = generate_training_data(training_samples);
//     auto targets = generate_targets(training_data);
//
//     // Train the LSTM
//     lstm.train(training_data, targets, 2000);
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
//         double predicted_prob = lstm.forward(test_input);
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
