# lin

> A command-line calculator for linear algebra, written in C.

`lin`, short for `linear-utils`, implements an expressive linear algebra calculator that accepts scalar values, inline matrix literals, matrix algebra, and computations such as determinant, inverses, and row reduction,
all from the command line.

## Installation

The easiest way to install `lin` is to clone this repository and build it from source using `make`.
```sh
git clone https://github.com/mateo-patino/linear-utils.git
cd linear-utils
make release

```

Optionally, add it to your local binaries to run it from anywhere in your system.
```sh
sudo cp lin /usr/local/bin/ 
```

## Usage

`lin` evaluates expressions as quoted command-line arguments. Expressions can contain scalars, matrices, and combinations of both.

### Scalar arithmetic

```sh
./lin "1 + 2 * 3"
# 7

./lin "( 1 + 2 ) * 3"
# 9

./lin "10 / 4"
# 2.5
```

All mathematical operators have word aliases, which can make expressions more readable in scripts:

```sh
./lin "2 plus 3 times 4"
./lin "10 over 2"
```

### Matrix arithmetic

Matrix literals use the following row-major form:

```text
<rows>x<columns> <entry-1> <entry-2> ...
```

For example, this is a 2-by-3 matrix:

```text
2x3 1 2 3 4 5 6
```

Matrices with matching dimensions can be added or subtracted directly:

```sh
./lin "2x2 1 2 3 4 + 2x2 5 6 7 8"
```

```text
| 6  8 |
| 10 12 |
```

Scalar multiplication can be applied to an entire matrix:

```sh
./lin "3 * 2x2 1 2 3 4"
```

```text
|  3  6 |
|  9 12 |
```

Matrix multiplication uses the same `*` operator. The dimensions must be
compatible, just as they are in conventional linear algebra:

```sh
./lin "2x2 1 2 3 4 * 2x2 5 6 7 8"
```

```text
| 19 22 |
| 43 50 |
```

### Determinants, inverses, and RREF

Unary matrix operations can be written before their matrix operand. The
available unary operations (and their aliases) are `det`/`determinant` (compute the determinant of a matrix), `inv`/`inverse` (compute the inverse matrix), 
and `rref`/`reduced` (compute the reduced-row echelon form):

```sh
./lin "det 2x2 1 2 3 4"
# -2

./lin "inv 2x2 1 2 3 4"
# | -2.000  1.000 |
# |  1.500 -0.500 |

./lin "rref 2x3 1 2 3 2 4 6"
# | 1 2 3 |
# | 0 0 0 |
```

Unary operations can be nested and combined with ordinary arithmetic. For example,
this computes the inverse of a matrix and then multiplies it by a column vector:

```sh
./lin "inv 2x2 1 2 3 4 * 2x1 5 6"
```

Expressions can also be grouped explicitly when composing several operations:

```sh
./lin "( inv 2x2 1 2 3 4 ) * 2x1 5 6"
```

All input is provided as one shell argument. Quoting the expression is
important because the shell otherwise interprets spaces and operators before
`lin` receives them.

## Program Architecture

`lin` is organized as a sequence of layers, with each stage transforming the
user's expression into the result printed by the command-line interface:

- **Command-line interface — `src/main.c`:** Accepts one expression, coordinates the pipeline, reports errors, and prints the final result.
- **Lexing — `src/lexer/`:** Converts scalar literals, matrix literals, operators, and parentheses into a validated token stream.
- **Parsing and AST — `src/parser/`:** Builds an abstract syntax tree (AST) using operator precedence, associativity, and explicit grouping.
- **Semantic analysis — `src/semantic/`:** Validates operand types, matrix dimensions, finite values, and other mathematical/matrix requirements.
- **Evaluation — `src/evaluator/`:** Traverses validated ASTs and dispatches scalar and matrix operations to the appropriate routines.
- **Linear algebra library — `src/linalg/`:** Provides matrix views, arithmetic kernels, row operations, RREF, determinants, and Gauss-Jordan inversion. This library is completely agnostic of the CLI.
- **Core data types — `src/types/`:** Defines tokens, matrices, and related objects shared across the frontend and evaluation layers.
- **Printing — `src/printer/`:** Formats scalar and matrix results for readable terminal output.
- **Memory management — `src/arena.*`:** Provides arena allocation for intermediate evaluation objects and parsed data.
- **Application test suite — `tests/`:** Contains frontend, parser, semantic, evaluator, and printer-oriented tests written in C.
- **Linear algebra tests — `tests/linalg/`:** Uses C++'s Eigen library to validate the numerical behavior of `src/linalg/` in isolation of the CLI. 
- **Memory tests — `tests/memory/`:** Supplies end-to-end expressions to the Valgrind-based memory-checking script.


## Testing

Run the unit-test suite:

```sh
make test
make test-linalg
```
Note that you may need to reinitialize the Git repository to include the Eigen dependency which
`test-linalg` requires.

Run the Valgrind-based memory checks after building the application (Linux environment needed):

```sh
make memcheck
```

The memory-check script reads expressions from files in `tests/memory/`.


## Contributing

Contributions and pull requests are welcome, especially in these areas:

- Semantic analysis for dimensions propagated through nested expressions (search for NEEDSWORK tags)
- Parser and lexer edge-case coverage (I've extensively tested these but more tests are welcome)
- Cross-platform testing and memory-safety improvements (don't have GitHub actions yet)
- Unit tests for the evaluator layer
- More tests for the linalg library

Before opening a PR, please keep changes focused, add or update relevant tests, and run:

```sh
make test
```

If your change affects allocation, ownership, or error paths, please also run:

```sh
make memcheck
```

