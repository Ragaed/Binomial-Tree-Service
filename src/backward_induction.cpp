#include "backward_induction.h"

#include <cmath>
#include <stdexcept>

namespace {

void validate_valuation_inputs(const RateLattice& rate_lattice,
                               const std::vector<double>& terminal_payoffs,
                               double down_move_probability) {
    if (!std::isfinite(down_move_probability)
        || down_move_probability < 0.0
        || down_move_probability > 1.0) {
        throw std::invalid_argument(
            "down-move probability must be between 0 and 1");
    }
    if (terminal_payoffs.empty()) {
        throw std::invalid_argument("terminal payoffs must not be empty");
    }
    if (rate_lattice.levels() < terminal_payoffs.size() - 1) {
        throw std::invalid_argument("rate lattice is too shallow for terminal payoffs");
    }
}

std::vector<double> calculate_previous_level_values(
    const RateLattice& rate_lattice,
    const std::vector<double>& next_level_values,
    std::size_t current_level,
    double down_move_probability) {
    std::vector<double> current_level_values(current_level + 1);
    const std::vector<double>& current_level_rates =
        rate_lattice.nodes_at(current_level);

    // Each value uses the rate at its matching (level, node) coordinate.
    for (std::size_t node = 0; node <= current_level; ++node) {
        const double discount_denominator = 1.0 + current_level_rates.at(node);
        if (discount_denominator <= 0.0) {
            throw std::invalid_argument("rate produces a non-positive discount denominator");
        }
        current_level_values[node] = (
            down_move_probability * next_level_values[node + 1]
            + (1.0 - down_move_probability) * next_level_values[node])
            / discount_denominator;
    }
    return current_level_values;
}

}  // namespace

BackwardInductionEngine::BackwardInductionEngine(double down_move_probability)
    : down_move_probability_(down_move_probability) {
    if (!std::isfinite(down_move_probability_)
        || down_move_probability_ < 0.0
        || down_move_probability_ > 1.0) {
        throw std::invalid_argument(
            "down-move probability must be between 0 and 1");
    }
}

double BackwardInductionEngine::present_value(
    const RateLattice& rate_lattice,
    const std::vector<double>& terminal_payoffs) const {
    validate_valuation_inputs(
        rate_lattice, terminal_payoffs, down_move_probability_);

    // Keep only the current slice so working memory remains O(N).
    std::vector<double> current_level_values = terminal_payoffs;
    for (std::size_t level = terminal_payoffs.size() - 1; level > 0; --level) {
        current_level_values = calculate_previous_level_values(
            rate_lattice,
            current_level_values,
            level - 1,
            down_move_probability_);
    }
    return current_level_values.front();
}

std::vector<std::vector<double>> BackwardInductionEngine::valuation_lattice(
    const RateLattice& rate_lattice,
    const std::vector<double>& terminal_payoffs) const {
    validate_valuation_inputs(
        rate_lattice, terminal_payoffs, down_move_probability_);

    const std::size_t horizon = terminal_payoffs.size() - 1;
    std::vector<std::vector<double>> valuation_levels(horizon + 1);
    valuation_levels[horizon] = terminal_payoffs;

    // Retain every computed slice for diagnostics and visualization.
    for (std::size_t level = horizon; level > 0; --level) {
        valuation_levels[level - 1] = calculate_previous_level_values(
            rate_lattice,
            valuation_levels[level],
            level - 1,
            down_move_probability_);
    }
    return valuation_levels;
}