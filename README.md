# lin

> A command-line calculator for linear algebra, written in C.

`lin`, short for `linear-utils`, implements an expressive linear algebra language that accepts scalar values, inline matrix literals, matrix algebra, and computations such as determinant, inverses, and row reduction,
all in a command-line format.

> [!WARNING]
> **This project is actively under development.** The APIs, linear algebra routines, and the overall architecture of the program may change as I work on it.

## Highlights

Some features and components that have been implemented so far:

- Scalar math with standard arithmetic precedence and associativity
- Inline matrix literal parsing. Strings such as `2x2 1 2 3 4` are parsed to a 2x2 matrix with those entries in a row-major layout.
- Abstract syntax tree (AST) and recursive descent parser
- Recursive semantic (mathematical) validation of the AST
- Custom linear algebra kernels (elementary row operations, Gauss-Jordan elimination, etc.) from scratch
- Evaluation/glue layer that connects the linear algebra functions to the rest of the program.
- Memory arena allocator
- Unit tests for lexer, parser, and semantic validation
- Valgrind memory-sanity tests for end-to-end executions

## Example syntax

Expressions are passed as one quoted command-line argument:

```sh
./lin "1 + 2 * 3"
./lin "2x2 1 2 3 4 + 2x2 5 6 7 8"
./lin "det 2x2 1 2 3 4"
./lin "rref ( inv 2x2 1 2 3 4 )"
./lin "( 2 * 2x2 1 0 0 1 ) + 2x2 5 6 7 8"
```

Matrix literals use this form:

```text
<rows>x<columns> <entry-1> <entry-2> ...
```

For example, this is a 2-by-3 matrix:

```text
2x3 1 2 3 4 5 6
```

## Supported language features

| Feature | Examples |
| --- | --- |
| Scalars | `1`, `-3.5`, `.25`, `2e3` |
| Matrices | `2x2 1 2 3 4` |
| Addition | `add`, `plus`, `+` |
| Subtraction | `sub`, `minus`, `-` |
| Multiplication | `mul`, `times`, `*` |
| Division | `div`, `over`, `/` |
| Determinant | `det`, `determinant`, `detof` |
| Row reduction | `rref`, `reduced` |
| Inverse | `inv`, `inverse` |
| Grouping  | `( ... )` |

`ADD`, `SUB`, and `MUL` are overloaded by the semantic layer: depending on their operands, they may represent scalar arithmetic, matrix arithmetic, or scalar-matrix multiplication. Division is scalar-only.


## Development status

If you know C and some linear algebra, please feel free to make a push request! A ton of work remains to be done. 

### Completed or substantially implemented

- [x] Token model for scalars, matrices, operators, and parentheses
- [x] Scalar and matrix-literal lexing
- [x] Operator aliases and precedence metadata
- [x] AST construction with precedence, associativity, and parenthesis handling
- [x] Semantic checks for operand types, matrix dimensions, finite values, and square-matrix requirements
- [x] Evaluation layer for reading the AST and dispatching operations to the linear algebra library 
- [x] Unit-test harness with isolated test execution
- [x] Lexer, parser, and semantic test suites
- [x] Memory-check script using Valgrind
- [x] Elementary row operations, Gauss-Jordan elimination, and convertion to upper-triangular

### In progress

- [ ] Pretty printing module capable of displaying evaluated results cleanly to the terminal
- [ ] Linear algebra test suite, likely to be implemented in C++ using Eigen to check for numerical correctness
- [ ] Evaluation layer test suite

## Building

`lin` uses `make` and a GCC-compatible C compiler. The Makefile prefers C23 when available and falls back to C2x.

```sh
make
```

This produces:

```text
./lin
```

## Testing

Run the unit-test suite:

```sh
make test
```

Run the Valgrind-based memory checks after building the application:

```sh
make memcheck
```

The memory-check script reads expressions from files in `tests/memory/`.

## Contributing

Contributions and pull requests are welcome, especially in these areas:

- Semantic analysis for dimensions propagated through nested expressions (search for NEEDSWORK tags)
- Parser and lexer edge-case coverage (I've extensively tested these but more tests are welcome)
- A pretty printing module for displaying matrices to the terminal. This is the last module of the main program, which I am about to start working on.
- Cross-platform testing and memory-safety improvements (don't have GitHub actions yet)
- Documentation, examples, and CLI usability

Before opening a PR, please keep changes focused, add or update relevant tests, and run:

```sh
make test
```

If your change affects allocation, ownership, or error paths, please also run:

```sh
make memcheck
```

