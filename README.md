# Binomial Tree Service

An educational C++ project for valuing cash flows on recombining binomial lattices. The project will eventually expose the valuation engine through a stateless REST API.

## Current MVP slice

The current slice constructs a rate lattice from:

- A strictly positive initial rate
- Strictly positive up and down factors
- A strictly positive number of steps

At each level, nodes are ordered from all-up to all-down. Rates are stored internally as decimal values, so 5 percent is represented as `0.05`.

## Build and run the tests

The current test executable uses only the C++ standard library and `g++`:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic `
  src/rate_lattice.cpp `
  tests/rate_lattice_test.cpp `
  -Isrc `
  -o rate_lattice_test.exe
./rate_lattice_test.exe
```

Run the command from the repository root, the directory containing `src` and `tests`.

The executable produces no output when all assertions pass. A failed assertion reports the source line that detected the failure.

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
