#pragma once

#include <vector>

#include "layered_lattice.h"

class BackwardInductionEngine {
public:
    explicit BackwardInductionEngine(double down_move_probability = 0.5);

    double present_value(const BaseLattice& rate_lattice,
                         const std::vector<double>& terminal_payoffs) const;

    LayeredLattice valuation_lattice(
        const BaseLattice& rate_lattice,
        const std::vector<double>& terminal_payoffs) const;

private:
    double down_move_probability_;
};