#pragma once

#include <vector>

#include "base_lattice.h"

class RateLattice : public BaseLattice {
public:
    // Rates are ordered from all-up to all-down at each level.
    RateLattice(double initial_rate, double up_factor, double down_factor, int steps);

    // Number of levels includes the initial level.
    std::size_t levels() const override;

    // Throws std::out_of_range when level is outside the lattice.
    const Layer& nodes_at(std::size_t level) const override;

private:
    std::vector<std::vector<double>> nodes_;
};
