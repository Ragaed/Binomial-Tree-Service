# Binomial Tree Service

An educational C++ project for valuing cash flows on recombining binomial lattices. The project will eventually expose the valuation engine through a stateless REST API.

## Current MVP slice

The current slice constructs a rate lattice from:

- A strictly positive initial rate
- Strictly positive up and down factors
- A strictly positive number of steps

At each level, nodes are ordered from all-up to all-down. Rates are stored internally as decimal values, so 5 percent is represented as `0.05`.

## Backward induction behavior

The pricing engine accepts a complete cash-flow lattice and a pre-built
short-rate lattice. The rate lattice may contain more levels than the cash-flow
tree, but it must contain every rate level needed for discounting. Both
structures use the same `(i, j)` node coordinates, including the shared
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

- Given a flat rate lattice and a final cash flow of `1.0`, rolling back `N`
	periods returns `(1 + r)^(-N)` within machine precision.
- Given a larger rate lattice, the engine uses only the matching rate nodes
	required by the payoff horizon and ignores extra levels.
- Given a cash-flow lattice with `N + 1` levels, level `i` contains
	exactly `i + 1` valuation nodes.
- Given full-lattice storage, the engine retains every valuation level for
	diagnostics and visualization.
- Given rolling-slice storage, the engine retains only the current slice and
	uses O(N) working memory.
- Given a probability outside `[0.0, 1.0]`, an insufficient rate lattice, or
	invalid payoff dimensions, the engine rejects the request.

## Base lattice shape contract

`BaseLattice` defines the shared geometry contract for lattice models without
defining model-specific rollback behavior. Each layer is a one-dimensional
vector, and layer `k` contains exactly `k + 1` nodes. Concrete models provide
their own layer values while consumers can use the common interface:

- Given a `BaseLattice` implementation, requesting `nodes_at(level)` returns
	the ordered state vector for that level.
- Given a valid level and node index, `value_at(level, node)` returns the value
	at that coordinate.
- Given an invalid level or node index, `nodes_at` or `value_at` rejects the
	request with `std::out_of_range`.
- Given a `RateLattice` through a `BaseLattice` reference, layer and node
	access produce the same geometry and values as concrete access.
- Given a valid cash-flow lattice and a `RateLattice`, backward induction keeps
	its existing present-value and valuation-lattice results.
- Given a valid cash-flow lattice, `valuation_lattice` returns a concrete
	`LayeredLattice` with the shared layer and node accessors.
- Given cash-flow levels deeper than the rate lattice, pricing rejects the invalid
	shape with `std::invalid_argument`.

## Cash-flow lattice behavior

The backward engine can price a complete `CashFlowLattice`, where each node
contains the cash flow paid at that time and state. Intermediate coupons,
floating payments, amortization, and terminal redemption all use the same
`(i, j)` coordinates as the rate lattice.

The final layer contains the complete maturity payment. For example, a final
coupon and principal redemption are stored together at step `N`; the engine
does not receive or add a separate terminal payoff when using this API. The
rollback equations are:

```text
V[N,j] = C[N,j]
V[i,j] = (q * V[i+1,j+1] + (1-q) * V[i+1,j]) / (1 + r[i,j]) + C[i,j]
```

This ensures that a cash flow at step `i` is received at that boundary and
that each maturity payment is included exactly once. A zero-valued terminal
layer prices intermediate cash flows only. With a flat rate and deterministic
cash flows, a coupon bond satisfies:

```text
P[0] = sum(C[k] / (1 + r)^k) for k = 1..N
```

`CashFlowLattice` reuses the existing validated binomial lattice shape. Its
`add_terminal_values` operation adds redemption or other terminal values to
the final layer without replacing existing final coupons. Sparse schedules
are represented by zero-valued nodes and require no special pricing path.

The cash-flow BDD scenarios are:

- Given a cash-flow lattice with a payment at step `N`, pricing returns its
	  discounted value.
- Given a flat rate lattice and coupon payments at steps `1..N`, including
	  principal with the final coupon, pricing returns the coupon-bond sum.
- Given zero terminal values, pricing returns the present value of only the
	  intermediate cash flows.
- Given sparse zero-valued layers or nodes, pricing includes nonzero flows at
	  their matching coordinates without changing the result.
- Given terminal values added to a schedule, existing final coupons remain
	  present and the terminal values are included once.
- Given incompatible rate and cash-flow lattice shapes, pricing rejects the
	  request with `std::invalid_argument`.

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
2. Add backward induction for cash-flow lattices.
3. Add calculation traces.
4. Introduce a REST adapter while keeping the calculation engine independent.
5. Add alternative lattice-building strategies such as BDT.

## Learning approach

The project follows Red-Green-Refactor:

1. Write a test that describes one behavior and watch it fail.
2. Implement the smallest change that makes it pass.
3. Refactor for clarity while keeping the tests passing.
