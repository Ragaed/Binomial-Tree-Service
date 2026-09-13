#pragma once

#include "layered_lattice.h"

class BackwardInductionEngine {
public:
    explicit BackwardInductionEngine(double down_move_probability = 0.5);

    double present_value(const BaseLattice& rate_lattice,
                         const BaseLattice& cash_flow_lattice) const;

    LayeredLattice valuation_lattice(
        const BaseLattice& rate_lattice,
        const BaseLattice& cash_flow_lattice) const;

private:
    double down_move_probability_;
};