#include "rate_lattice.h"

#include <cmath>
#include <stdexcept>

namespace {

void validate_parameters(double initial_rate, double up_factor, double down_factor, int steps) {
    if (initial_rate <= 0.0) {
        throw std::invalid_argument("initial rate must be positive");
    }
    if (up_factor <= 0.0 || down_factor <= 0.0) {
        throw std::invalid_argument("factors must be positive");
    }
    if (steps <= 0) {
        throw std::invalid_argument("steps must be positive");
    }
}

}  // namespace

RateLattice::RateLattice(double initial_rate, double up_factor, double down_factor, int steps)
    : nodes_() {
    validate_parameters(initial_rate, up_factor, down_factor, steps);

    nodes_.resize(static_cast<std::size_t>(steps + 1));
    nodes_[0].push_back(initial_rate);

    // At level t, node j represents t - j up moves and j down moves.
    // Because multiplication commutes, up-then-down and down-then-up
    // arrive at the same value, so the lattice recombines.
    for (int level = 1; level <= steps; ++level) {
        nodes_[level].reserve(level + 1);
        for (int down_moves = 0; down_moves <= level; ++down_moves) {
            const double rate = initial_rate
                * std::pow(up_factor, level - down_moves)
                * std::pow(down_factor, down_moves);
            nodes_[level].push_back(rate);
        }
    }
}

std::size_t RateLattice::levels() const {
    return nodes_.size();
}

const std::vector<double>& RateLattice::nodes_at(std::size_t level) const {
    return nodes_.at(level);
}
