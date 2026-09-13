#include "backward_induction.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace {

void validate_cash_flow_inputs(const BaseLattice& rate_lattice,
                               const BaseLattice& cash_flow_lattice,
                               double down_move_probability) {
    if (!std::isfinite(down_move_probability)
        || down_move_probability < 0.0
        || down_move_probability > 1.0) {
        throw std::invalid_argument(
            "down-move probability must be between 0 and 1");
    }
    if (cash_flow_lattice.levels() == 0) {
        throw std::invalid_argument("cash-flow lattice must not be empty");
    }
    if (rate_lattice.levels() < cash_flow_lattice.levels()) {
        throw std::invalid_argument(
            "rate lattice is too shallow for cash-flow lattice");
    }
}

std::vector<double> calculate_previous_level_values(
    const BaseLattice& rate_lattice,
    const std::vector<double>& next_level_values,
    std::size_t current_level,
    double down_move_probability,
    const BaseLattice* cash_flow_lattice = nullptr) {
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
        if (cash_flow_lattice != nullptr) {
            current_level_values[node] +=
                cash_flow_lattice->value_at(current_level, node);
        }
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
    const BaseLattice& rate_lattice,
    const BaseLattice& cash_flow_lattice) const {
    validate_cash_flow_inputs(
        rate_lattice, cash_flow_lattice, down_move_probability_);

    const std::size_t horizon = cash_flow_lattice.levels() - 1;
    std::vector<double> current_level_values =
        cash_flow_lattice.nodes_at(horizon);
    for (std::size_t level = horizon; level > 0; --level) {
        current_level_values = calculate_previous_level_values(
            rate_lattice,
            current_level_values,
            level - 1,
            down_move_probability_,
            &cash_flow_lattice);
    }
    return current_level_values.front();
}

LayeredLattice BackwardInductionEngine::valuation_lattice(
    const BaseLattice& rate_lattice,
    const BaseLattice& cash_flow_lattice) const {
    validate_cash_flow_inputs(
        rate_lattice, cash_flow_lattice, down_move_probability_);

    const std::size_t horizon = cash_flow_lattice.levels() - 1;
    std::vector<std::vector<double>> valuation_levels(horizon + 1);
    valuation_levels[horizon] = cash_flow_lattice.nodes_at(horizon);

    for (std::size_t level = horizon; level > 0; --level) {
        valuation_levels[level - 1] = calculate_previous_level_values(
            rate_lattice,
            valuation_levels[level],
            level - 1,
            down_move_probability_,
            &cash_flow_lattice);
    }
    return LayeredLattice(std::move(valuation_levels));
}