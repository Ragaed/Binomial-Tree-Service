#pragma once

#include <stdexcept>
#include <utility>
#include <vector>

#include "base_lattice.h"

class LayeredLattice : public BaseLattice {
public:
    explicit LayeredLattice(std::vector<Layer> layers)
        : layers_(std::move(layers)) {
        // Validate that each layer has the expected binomial shape.
        for (std::size_t level = 0; level < layers_.size(); ++level) {
            if (layers_[level].size() != level + 1) {
                throw std::invalid_argument(
                    "lattice layer has an invalid number of nodes");
            }
        }
    }

    std::size_t levels() const override {
        return layers_.size();
    }

    const Layer& nodes_at(std::size_t level) const override {
        return layers_.at(level);
    }

private:
    std::vector<Layer> layers_;
};