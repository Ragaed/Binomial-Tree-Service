#include <cassert>
#include <cmath>
#include <stdexcept>

#include "rate_lattice.h"

void expect_near(double actual, double expected) {
    assert(std::fabs(actual - expected) < 1e-9);
}

void constructs_one_step_lattice() {
    RateLattice lattice(0.05, 1.1, 0.9, 1);

    assert(lattice.levels() == 2);
    assert(lattice.nodes_at(0).size() == 1);
    assert(lattice.nodes_at(1).size() == 2);
    expect_near(lattice.nodes_at(0).at(0), 0.05);
    expect_near(lattice.nodes_at(1).at(0), 0.055);
    expect_near(lattice.nodes_at(1).at(1), 0.045);
}

void constructs_two_step_recombining_lattice() {
    RateLattice lattice(0.05, 1.1, 0.9, 2);

    assert(lattice.levels() == 3);
    assert(lattice.nodes_at(2).size() == 3);
    expect_near(lattice.nodes_at(2).at(0), 0.0605);
    expect_near(lattice.nodes_at(2).at(1), 0.0495);
    expect_near(lattice.nodes_at(2).at(2), 0.0405);
    expect_near(0.05 * 1.1 * 0.9, lattice.nodes_at(2).at(1));
}

void rejects_non_positive_parameters() {
    bool rejected_rate = false;
    try {
        RateLattice lattice(0.0, 1.1, 0.9, 1);
    } catch (const std::invalid_argument&) {
        rejected_rate = true;
    }
    assert(rejected_rate);

    bool rejected_factor = false;
    try {
        RateLattice lattice(0.05, -1.1, 0.9, 1);
    } catch (const std::invalid_argument&) {
        rejected_factor = true;
    }
    assert(rejected_factor);

    bool rejected_steps = false;
    try {
        RateLattice lattice(0.05, 1.1, 0.9, 0);
    } catch (const std::invalid_argument&) {
        rejected_steps = true;
    }
    assert(rejected_steps);
}

int main() {
    constructs_one_step_lattice();
    constructs_two_step_recombining_lattice();
    rejects_non_positive_parameters();
    return 0;
}
