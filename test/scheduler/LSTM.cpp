#include "LSTM.h"

LSTM::LSTM()
{
    std::mt19937 gen(static_cast<unsigned>(time(nullptr)));
    std::normal_distribution<double> dist(0.0, 0.1);

    // Initialize weights and biases
    Wf.resize(input_size, std::vector<double>(hidden_size));
    Wi.resize(input_size, std::vector<double>(hidden_size));
    Wc.resize(input_size, std::vector<double>(hidden_size));
    Wo.resize(input_size, std::vector<double>(hidden_size));
    Uf.resize(hidden_size, std::vector<double>(hidden_size));
    Ui.resize(hidden_size, std::vector<double>(hidden_size));
    Uc.resize(hidden_size, std::vector<double>(hidden_size));
    Uo.resize(hidden_size, std::vector<double>(hidden_size));
    bf.resize(hidden_size);
    bi.resize(hidden_size);
    bc.resize(hidden_size);
    bo.resize(hidden_size);

    // Initialize weights
    for (int i = 0; i < input_size; ++i) {
        for (int j = 0; j < hidden_size; ++j) {
            Wf[i][j] = dist(gen);
            Wi[i][j] = dist(gen);
            Wc[i][j] = dist(gen);
            Wo[i][j] = dist(gen);
        }
    }
    for (int i = 0; i < hidden_size; ++i) {
        for (int j = 0; j < hidden_size; ++j) {
            Uf[i][j] = dist(gen);
            Ui[i][j] = dist(gen);
            Uc[i][j] = dist(gen);
            Uo[i][j] = dist(gen);
        }
        bf[i] = dist(gen);
        bi[i] = dist(gen);
        bc[i] = dist(gen);
        bo[i] = dist(gen);
    }

    // Output layer weights
    Wout.resize(hidden_size);
    for (int i = 0; i < hidden_size; ++i) {
        Wout[i] = dist(gen);
    }
    bout = dist(gen);
}

void LSTM::forward_step(const std::vector<double>& x,
                        std::vector<double>& h_prev,
                        std::vector<double>& c_prev,
                        std::vector<double>& h,
                        std::vector<double>& c,
                        std::vector<double>& f,
                        std::vector<double>& i,
                        std::vector<double>& g,
                        std::vector<double>& o)
{
    h.resize(hidden_size);
    c.resize(hidden_size);
    f.resize(hidden_size);
    i.resize(hidden_size);
    g.resize(hidden_size);
    o.resize(hidden_size);

    // Compute gates
    for (int j = 0; j < hidden_size; ++j) {
        double f_sum = bf[j], i_sum = bi[j], c_sum = bc[j], o_sum = bo[j];
        for (int k = 0; k < input_size; ++k) {
            f_sum += Wf[k][j] * x[k];
            i_sum += Wi[k][j] * x[k];
            c_sum += Wc[k][j] * x[k];
            o_sum += Wo[k][j] * x[k];
        }
        for (int k = 0; k < hidden_size; ++k) {
            f_sum += Uf[k][j] * h_prev[k];
            i_sum += Ui[k][j] * h_prev[k];
            c_sum += Uc[k][j] * h_prev[k];
            o_sum += Uo[k][j] * h_prev[k];
        }
        f[j] = sigmoid(f_sum);
        i[j] = sigmoid(i_sum);
        g[j] = tanh(c_sum);
        o[j] = sigmoid(o_sum);
        c[j] = f[j] * c_prev[j] + i[j] * g[j];
        h[j] = o[j] * tanh(c[j]);
    }
}

auto LSTM::forward(const std::vector<double>& inputs) -> double
{
    std::vector<double> h(hidden_size, 0.0), c(hidden_size, 0.0);
    std::vector<double> h_prev(hidden_size, 0.0), c_prev(hidden_size, 0.0);
    std::vector<double> f, i, g, o;

    // Process inputs as a sequence
    forward_step(inputs, h_prev, c_prev, h, c, f, i, g, o);

    // Compute output
    double output_sum = bout;
    for (int j = 0; j < hidden_size; ++j) {
        output_sum += Wout[j] * h[j];
    }
    return sigmoid(output_sum);
}

void LSTM::train(const std::vector<std::vector<double>>& inputs,
                 const std::vector<double>& targets,
                 int epochs)
{
    std::mt19937 gen(static_cast<unsigned>(time(0)));
    std::normal_distribution<double> dist(0.0, 0.01);

    for (int epoch = 0; epoch < epochs; ++epoch) {
        double total_error = 0.0;
        for (size_t i = 0; i < inputs.size(); ++i) {
            std::vector<double> h(hidden_size, 0.0), c(hidden_size, 0.0);
            std::vector<double> h_prev(hidden_size, 0.0), c_prev(hidden_size, 0.0);
            std::vector<double> f, k, g, o;

            // Forward pass
            forward_step(inputs[i], h_prev, c_prev, h, c, f, k, g, o);
            double output_sum = bout;
            for (int j = 0; j < hidden_size; ++j) {
                output_sum += Wout[j] * h[j];
            }
            double output = sigmoid(output_sum);

            // Compute error
            double error = targets[i] - output;
            total_error += error * error;

            // Backpropagation
            double output_delta = error * sigmoid_derivative(output_sum);

            // Update output weights
            for (int j = 0; j < hidden_size; ++j) {
                Wout[j] += learning_rate * output_delta * h[j];
            }
            bout += learning_rate * output_delta;

            // Compute gradients for LSTM gates
            std::vector<double> dh(hidden_size, 0.0);
            std::vector<double> dc(hidden_size, 0.0);
            for (int j = 0; j < hidden_size; ++j) {
                dh[j] = output_delta * Wout[j] * tanh_derivative(c[j]);
                dc[j] = dh[j] * o[j] * tanh_derivative(c[j]);
            }

            // Update gate weights and biases
            for (int j = 0; j < hidden_size; ++j) {
                double df = dc[j] * c_prev[j] * sigmoid_derivative(f[j]);
                double di = dc[j] * g[j] * sigmoid_derivative(k[j]);
                double dg = dc[j] * k[j] * tanh_derivative(g[j]);
                double d_o = dh[j] * tanh(c[j]) * sigmoid_derivative(o[j]);

                for (int k = 0; k < input_size; ++k) {
                    Wf[k][j] += learning_rate * df * inputs[i][k];
                    Wi[k][j] += learning_rate * di * inputs[i][k];
                    Wc[k][j] += learning_rate * dg * inputs[i][k];
                    Wo[k][j] += learning_rate * d_o * inputs[i][k];
                }
                for (int k = 0; k < hidden_size; ++k) {
                    Uf[k][j] += learning_rate * df * h_prev[k];
                    Ui[k][j] += learning_rate * di * h_prev[k];
                    Uc[k][j] += learning_rate * dg * h_prev[k];
                    Uo[k][j] += learning_rate * d_o * h_prev[k];
                }
                bf[j] += learning_rate * df;
                bi[j] += learning_rate * di;
                bc[j] += learning_rate * dg;
                bo[j] += learning_rate * d_o;
            }
        }
    }
}
