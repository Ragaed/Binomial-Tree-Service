#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "backward_induction.h"
#include "base_lattice.h"
#include "cash_flow_lattice.h"
#include "rate_lattice.h"

void expect_near(double actual, double expected) {
    assert(std::fabs(actual - expected) < 1e-9);
}

CashFlowLattice terminal_cash_flows(const std::vector<double>& terminal_values) {
    std::vector<BaseLattice::Layer> layers(terminal_values.size());
    for (std::size_t level = 0; level < layers.size(); ++level) {
        layers[level].resize(level + 1, 0.0);
    }
    layers.back() = terminal_values;
    return CashFlowLattice(std::move(layers));
}

void constructs_one_step_lattice() {
    // Arrange
    RateLattice lattice(0.05, 1.1, 0.9, 1);

    // Act
    const std::size_t number_of_levels = lattice.levels();
    const std::vector<double>& initial_level = lattice.nodes_at(0);
    const std::vector<double>& one_step_level = lattice.nodes_at(1);

    // Assert
    assert(number_of_levels == 2);
    assert(initial_level.size() == 1);
    assert(one_step_level.size() == 2);
    expect_near(initial_level.at(0), 0.05);
    expect_near(one_step_level.at(0), 0.055);
    expect_near(one_step_level.at(1), 0.045);
}

void accesses_rate_lattice_through_base_interface() {
    // Arrange
    RateLattice lattice(0.05, 1.1, 0.9, 1);
    const BaseLattice& base_lattice = lattice;

    // Act
    const BaseLattice::Layer& level = base_lattice.nodes_at(1);
    const double up_rate = base_lattice.value_at(1, 0);
    const double down_rate = base_lattice.value_at(1, 1);

    // Assert
    assert(level.size() == 2);
    expect_near(up_rate, 0.055);
    expect_near(down_rate, 0.045);
}

void rejects_invalid_base_lattice_coordinates() {
    // Arrange
    RateLattice lattice(0.05, 1.1, 0.9, 1);
    const BaseLattice& base_lattice = lattice;
    bool rejected_level = false;
    bool rejected_node = false;

    // Act
    try {
        base_lattice.nodes_at(2);
    } catch (const std::out_of_range&) {
        rejected_level = true;
    }
    try {
        base_lattice.value_at(1, 2);
    } catch (const std::out_of_range&) {
        rejected_node = true;
    }

    // Assert
    assert(rejected_level);
    assert(rejected_node);
}

void prices_through_base_lattice_interface() {
    // Arrange
    RateLattice lattice(0.05, 1.0, 1.0, 1);
    const BaseLattice& base_lattice = lattice;
    BackwardInductionEngine engine;
    const CashFlowLattice cash_flows = terminal_cash_flows({2.0, 4.0});

    // Act
    const double present_value = engine.present_value(
        base_lattice, cash_flows);

    // Assert
    expect_near(present_value, 3.0 / 1.05);
}

void prices_coupon_bond_from_cash_flow_lattice() {
    // Arrange
    constexpr double rate = 0.05;
    RateLattice rate_lattice(rate, 1.0, 1.0, 3);
    CashFlowLattice cash_flows({
        {0.0},
        {5.0, 5.0},
        {5.0, 5.0, 5.0},
        {105.0, 105.0, 105.0, 105.0}});
    BackwardInductionEngine engine;
    const double expected_present_value =
        5.0 / (1.0 + rate)
        + 5.0 / std::pow(1.0 + rate, 2)
        + 105.0 / std::pow(1.0 + rate, 3);

    // Act
    const double present_value = engine.present_value(
        rate_lattice, cash_flows);

    // Assert
    expect_near(present_value, expected_present_value);
}

void adds_terminal_values_without_replacing_final_cash_flows() {
    // Arrange
    CashFlowLattice cash_flows({
        {0.0},
        {5.0, 5.0},
        {5.0, 5.0, 5.0}});

    // Act
    cash_flows.add_terminal_values({100.0, 100.0, 100.0});

    // Assert
    expect_near(cash_flows.value_at(2, 0), 105.0);
    expect_near(cash_flows.value_at(2, 1), 105.0);
    expect_near(cash_flows.value_at(2, 2), 105.0);
}

void prices_intermediate_cash_flows_with_zero_terminal_layer() {
    // Arrange
    constexpr double rate = 0.05;
    RateLattice rate_lattice(rate, 1.0, 1.0, 3);
    CashFlowLattice cash_flows({
        {0.0},
        {2.0, 2.0},
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0}});
    BackwardInductionEngine engine;

    // Act
    const double present_value = engine.present_value(
        rate_lattice, cash_flows);

    // Assert
    expect_near(present_value, 2.0 / (1.0 + rate));
}

void rejects_invalid_cash_flow_lattice_inputs() {
    // Arrange
    bool rejected_empty_lattice = false;
    bool rejected_terminal_shape = false;

    // Act
    try {
        CashFlowLattice cash_flows({});
    } catch (const std::invalid_argument&) {
        rejected_empty_lattice = true;
    }

    CashFlowLattice cash_flows({{0.0}, {1.0, 1.0}});
    try {
        cash_flows.add_terminal_values({100.0});
    } catch (const std::invalid_argument&) {
        rejected_terminal_shape = true;
    }

    // Assert
    assert(rejected_empty_lattice);
    assert(rejected_terminal_shape);
}

void rejects_invalid_layered_lattice_shape() {
    // Arrange
    bool rejected_shape = false;

    // Act
    try {
        LayeredLattice lattice({BaseLattice::Layer{1.0, 2.0}});
    } catch (const std::invalid_argument&) {
        rejected_shape = true;
    }

    // Assert
    assert(rejected_shape);
}

void constructs_two_step_recombining_lattice() {
    // Arrange
    RateLattice lattice(0.05, 1.1, 0.9, 2);

    // Act
    const std::size_t number_of_levels = lattice.levels();
    const std::vector<double>& two_step_level = lattice.nodes_at(2);
    const double recombining_rate = 0.05 * 1.1 * 0.9;

    // Assert
    assert(number_of_levels == 3);
    assert(two_step_level.size() == 3);
    expect_near(two_step_level.at(0), 0.0605);
    expect_near(two_step_level.at(1), 0.0495);
    expect_near(two_step_level.at(2), 0.0405);
    expect_near(recombining_rate, two_step_level.at(1));
}

void rejects_non_positive_parameters() {
    // Arrange
    bool rejected_rate = false;
    bool rejected_factor = false;
    bool rejected_steps = false;

    // Act
    try {
        RateLattice lattice(0.0, 1.1, 0.9, 1);
    } catch (const std::invalid_argument&) {
        rejected_rate = true;
    }
    try {
        RateLattice lattice(0.05, -1.1, 0.9, 1);
    } catch (const std::invalid_argument&) {
        rejected_factor = true;
    }
    try {
        RateLattice lattice(0.05, 1.1, 0.9, 0);
    } catch (const std::invalid_argument&) {
        rejected_steps = true;
    }

    // Assert
    assert(rejected_rate);
    assert(rejected_factor);
    assert(rejected_steps);
}

void discounts_flat_zero_coupon_with_rolling_slice() {
    // Arrange
    constexpr int valuation_horizon = 3;
    constexpr double flat_rate = 0.05;
    RateLattice lattice(flat_rate, 1.0, 1.0, valuation_horizon);
    BackwardInductionEngine engine;
    const CashFlowLattice cash_flows = terminal_cash_flows(
        std::vector<double>(valuation_horizon + 1, 1.0));
    const double expected_discounted_value =
        std::pow(1.0 + flat_rate, -valuation_horizon);

    // Act
    const double zero_coupon_present_value =
        engine.present_value(lattice, cash_flows);

    // Assert
    expect_near(zero_coupon_present_value, expected_discounted_value);
}

void uses_matching_rate_nodes_and_supports_larger_rate_lattice() {
    // Arrange
    constexpr double root_rate = 0.05;
    constexpr double up_factor = 1.1;
    constexpr double down_factor = 0.9;
    constexpr int rate_lattice_horizon = 3;
    constexpr int valuation_horizon = 2;
    RateLattice rate_lattice(
        root_rate, up_factor, down_factor, rate_lattice_horizon);
    BackwardInductionEngine engine;
    const CashFlowLattice cash_flows = terminal_cash_flows(
        std::vector<double>(valuation_horizon + 1, 1.0));
    const double up_node_rate = root_rate * up_factor;
    const double down_node_rate = root_rate * down_factor;
    const double value_at_up_node = 1.0 / (1.0 + up_node_rate);
    const double value_at_down_node = 1.0 / (1.0 + down_node_rate);
    const double expected_zero_time_value =
        (0.5 * value_at_up_node + 0.5 * value_at_down_node)
        / (1.0 + root_rate);

    // Act
    const double zero_time_present_value =
        engine.present_value(rate_lattice, cash_flows);

    // Assert
    expect_near(zero_time_present_value, expected_zero_time_value);
}

void supports_configurable_probability() {
    // Arrange
    constexpr double rate = 0.05;
    constexpr double probability_of_down_move = 0.25;
    constexpr double probability_of_up_move = 1.0 - probability_of_down_move;
    constexpr double up_state_payoff = 2.0;
    constexpr double down_state_payoff = 4.0;
    RateLattice lattice(rate, 1.0, 1.0, 1);
    BackwardInductionEngine engine(probability_of_down_move);
    const CashFlowLattice cash_flows = terminal_cash_flows({
        up_state_payoff, down_state_payoff});
    const double expected_zero_time_value =
        (probability_of_down_move * down_state_payoff
         + probability_of_up_move * up_state_payoff)
        / (1.0 + rate);

    // Act
    const double zero_time_present_value =
        engine.present_value(lattice, cash_flows);

    // Assert
    expect_near(zero_time_present_value, expected_zero_time_value);
}

void stores_full_valuation_lattice() {
    // Arrange
    constexpr int valuation_horizon = 2;
    RateLattice lattice(0.05, 1.0, 1.0, valuation_horizon);
    BackwardInductionEngine engine;
    const CashFlowLattice cash_flows = terminal_cash_flows(
        std::vector<double>(valuation_horizon + 1, 1.0));

    // Act
    const BaseLattice& values = engine.valuation_lattice(
        lattice, cash_flows);
    const double rolling_slice_zero_time_value =
        engine.present_value(lattice, cash_flows);

    // Assert
    assert(values.levels() == 3);
    assert(values.nodes_at(0).size() == 1);
    assert(values.nodes_at(1).size() == 2);
    assert(values.nodes_at(2).size() == 3);
    expect_near(values.value_at(0, 0), rolling_slice_zero_time_value);
}

void rejects_invalid_pricing_inputs() {
    // Arrange
    constexpr double flat_rate = 0.05;
    constexpr double no_rate_change_up_factor = 1.0;
    constexpr double no_rate_change_down_factor = 1.0;
    constexpr int available_rate_steps = 1;
    constexpr double invalid_probability = 1.1;
    constexpr int requested_valuation_horizon = 3;
    const CashFlowLattice cash_flows = terminal_cash_flows(
        std::vector<double>(requested_valuation_horizon + 1, 1.0));
    RateLattice lattice(
        flat_rate,
        no_rate_change_up_factor,
        no_rate_change_down_factor,
        available_rate_steps);
    bool rejected_probability = false;
    bool rejected_payoff_dimensions = false;

    // Act
    try {
        BackwardInductionEngine engine(invalid_probability);
    } catch (const std::invalid_argument&) {
        rejected_probability = true;
    }
    try {
        BackwardInductionEngine engine;
        engine.present_value(lattice, cash_flows);
    } catch (const std::invalid_argument&) {
        rejected_payoff_dimensions = true;
    }

    // Assert
    assert(rejected_probability);
    assert(rejected_payoff_dimensions);
}

int main() {
    constructs_one_step_lattice();
    accesses_rate_lattice_through_base_interface();
    rejects_invalid_base_lattice_coordinates();
    prices_through_base_lattice_interface();
    prices_coupon_bond_from_cash_flow_lattice();
    adds_terminal_values_without_replacing_final_cash_flows();
    prices_intermediate_cash_flows_with_zero_terminal_layer();
    rejects_invalid_cash_flow_lattice_inputs();
    rejects_invalid_layered_lattice_shape();
    constructs_two_step_recombining_lattice();
    rejects_non_positive_parameters();
    discounts_flat_zero_coupon_with_rolling_slice();
    uses_matching_rate_nodes_and_supports_larger_rate_lattice();
    supports_configurable_probability();
    stores_full_valuation_lattice();
    rejects_invalid_pricing_inputs();
    return 0;
}
