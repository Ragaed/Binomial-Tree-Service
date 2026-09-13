# Binomial Tree Service

An educational C++ project for valuing cash flows on recombining binomial lattices. The project will eventually expose the valuation engine through a stateless REST API.

## Current MVP slice

The current slice constructs a rate lattice from:

- A strictly positive initial rate
- Strictly positive up and down factors
- A strictly positive number of steps

At each level, nodes are ordered from all-up to all-down. Rates are stored internally as decimal values, so 5 percent is represented as `0.05`.

## Backward induction behavior

The pricing engine accepts a terminal payoff vector at step `N` and a
pre-built short-rate lattice. The rate lattice may contain more levels than
the payoff tree, but it must contain every rate level needed for discounting.
Both structures use the same `(i, j)` node coordinates, including the shared
`(0, 0)` root. The rate used at `(i, j)` is the rate stored at that exact rate
lattice node; the `(0, 0)` rate is not reused elsewhere.

For each node, the engine applies:

```text
V[i,j] = (q * V[i+1,j+1] + (1-q) * V[i+1,j]) / (1 + r[i,j])
```

The default down-move probability is `q = 0.5`, and callers may configure
another down-move probability between `0.0` and `1.0`. With nodes ordered from
all-up to all-down, `q` weights child `(i + 1, j + 1)`.

The behavior is defined by these BDD scenarios:

- Given a flat rate lattice and terminal payoff `1.0`, rolling back `N`
	periods returns `(1 + r)^(-N)` within machine precision.
- Given a larger rate lattice, the engine uses only the matching rate nodes
	required by the payoff horizon and ignores extra levels.
- Given a terminal payoff vector with `N + 1` values, level `i` contains
	exactly `i + 1` valuation nodes.
- Given full-lattice storage, the engine retains every valuation level for
	diagnostics and visualization.
- Given rolling-slice storage, the engine retains only the current slice and
	uses O(N) working memory.
- Given a probability outside `[0.0, 1.0]`, an insufficient rate lattice, or
	invalid payoff dimensions, the engine rejects the request.

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
