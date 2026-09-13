#pragma once

#include <stdexcept>
#include <utility>
#include <vector>

#include "layered_lattice.h"

class CashFlowLattice : public BaseLattice {
public:
    explicit CashFlowLattice(std::vector<Layer> layers)
        : lattice_(std::move(layers)) {
        if (lattice_.levels() == 0) {
            throw std::invalid_argument("cash-flow lattice must not be empty");
        }
    }

    std::size_t levels() const override {
        return lattice_.levels();
    }

    const Layer& nodes_at(std::size_t level) const override {
        return lattice_.nodes_at(level);
    }

    void add_terminal_values(const Layer& terminal_values) {
        const std::size_t terminal_level = lattice_.levels() - 1;
        if (terminal_values.size() != terminal_level + 1) {
            throw std::invalid_argument(
                "terminal values do not match cash-flow lattice shape");
        }

        std::vector<Layer> layers;
        layers.reserve(lattice_.levels());
        for (std::size_t level = 0; level < lattice_.levels(); ++level) {
            layers.push_back(lattice_.nodes_at(level));
        }
        for (std::size_t node = 0; node < terminal_values.size(); ++node) {
            layers[terminal_level][node] += terminal_values[node];
        }
        lattice_ = LayeredLattice(std::move(layers));
    }

private:
    LayeredLattice lattice_;
};
