#pragma once

#include <vector>

#include "rate_lattice.h"

class BackwardInductionEngine {
public:
    explicit BackwardInductionEngine(double down_move_probability = 0.5);

    double present_value(const RateLattice& rate_lattice,
                         const std::vector<double>& terminal_payoffs) const;

    std::vector<std::vector<double>> valuation_lattice(
        const RateLattice& rate_lattice,
        const std::vector<double>& terminal_payoffs) const;

private:
    double down_move_probability_;
};