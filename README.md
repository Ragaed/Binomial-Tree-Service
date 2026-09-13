# Binomial Tree Service

An educational C++ project for valuing cash flows on recombining binomial lattices. The project will eventually expose the valuation engine through a stateless REST API.

## Current MVP slice

The current slice constructs a rate lattice from:

- A strictly positive initial rate
- Strictly positive up and down factors
- A strictly positive number of steps

At each level, nodes are ordered from all-up to all-down. Rates are stored internally as decimal values, so 5 percent is represented as `0.05`.

## Build and run the tests

The project uses CMake and CTest. From the repository root, configure and build
the project with:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The build enables C++17, `-Wall`, `-Wextra`, `-Wpedantic`, and `-Werror` for
GCC and Clang. To run the tests with AddressSanitizer and
UndefinedBehaviorSanitizer enabled:

```powershell
cmake -S . -B build-sanitized -DENABLE_SANITIZERS=ON
cmake --build build-sanitized --parallel
ctest --test-dir build-sanitized --output-on-failure
```

The same build and test steps run automatically in GitHub Actions for GCC,
Clang, and the sanitizer configuration.

## Planned slices

1. Construct and test multi-step recombining lattices.
2. Add backward induction for terminal payoff vectors.
3. Add intermediate cash flows and calculation traces.
4. Introduce a REST adapter while keeping the calculation engine independent.
5. Add alternative lattice-building strategies such as BDT.

## Learning approach

The project follows Red-Green-Refactor:

1. Write a test that describes one behavior and watch it fail.
2. Implement the smallest change that makes it pass.
3. Refactor for clarity while keeping the tests passing.
