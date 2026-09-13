#pragma once

#include <cstddef>
#include <vector>

class BaseLattice {
public:
    using Layer = std::vector<double>;

    virtual ~BaseLattice() = default;

    virtual std::size_t levels() const = 0;
    virtual const Layer& nodes_at(std::size_t level) const = 0;

    double value_at(std::size_t level, std::size_t node) const {
        return nodes_at(level).at(node);
    }
};