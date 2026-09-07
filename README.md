# lin

> A (work-in-progress!) command-line calculator for linear algebra, written in C.

`lin` implements an expressive linear algebra language that accepts scalar values, inline matrix literals, arithmetic, matrix operations, and unary operations such as determinant, inverse, and row reduction,
all in a command-line format.

This is a highly modular project: user input moves through a lexer, parser, abstract syntax tree, semantic checker, evaluator, and a small linear algebra library.

> [!WARNING]
> **This project is actively under development.** The language, CLI output, APIs, and supported evaluation features may change as the evaluator and linear algebra layers mature. 

## Highlights

Some features and components that have been implemented so far:

- Scalar math with standard arithmetic precedence and associativity
- Inline matrix literal parsing. Strings such as `2x2 1 2 3 4` are parsed to a 2x2 matrix with those entries.
- Abstract syntax tree (AST) construction
- Semantic (mathematical) validation of the AST
- Custom linear algebra kernels from scratch
- Evaluation/glue layer that connects the linear algebra kernels to the rest of the program.
- Unit tests for lexer, parser, and semantic validation
- Valgrind-based memory-check workflow for end-to-end CLI inputs

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
| Division (WIP) | `div`, `over`, `/` |
| Determinant (WIP) | `det`, `determinant`, `detof` |
| Row reduction (WIP) | `rref`, `reduced` |
| Inverse (WIP) | `inv`, `inverse` |
| Grouping  | `( ... )` |

`ADD`, `SUB`, and `MUL` are overloaded by the semantic layer: depending on their operands, they may represent scalar arithmetic, matrix arithmetic, or scalar-matrix multiplication. Division is obviously scalar-only.


## Development status

If you know C and some linear algebra, please feel free to make a push request! A ton of work remains to be done. 

### Completed or substantially implemented

- [x] Token model for scalars, matrices, operators, and parentheses
- [x] Scalar and matrix-literal lexing
- [x] Operator aliases and precedence metadata
- [x] AST construction with precedence, associativity, and parenthesis handling
- [x] Semantic checks for operand types, matrix dimensions, finite values, and square-matrix requirements
- [x] Unit-test harness with isolated test execution
- [x] Lexer, parser, and semantic test suites
- [x] Memory-check script using Valgrind
- [x] Initial linear algebra view and arithmetic interfaces

### In progress

- [ ] Complete evaluator coverage for all operators
- [ ] Present evaluated results cleanly in the CLI
- [ ] Complete matrix addition, subtraction, multiplication, and scalar-matrix evaluation paths
- [ ] Determinant, inverse, and RREF evaluation
- [ ] Richer semantic propagation through nested matrix expressions
- [ ] Improved diagnostics with source-oriented expression context
- [ ] Broader test coverage for evaluator and linear algebra kernels

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

- Linear algebra algorithms: determinant, inverse, and RREF
- Semantic analysis for dimensions propagated through nested expressions (search for NEEDSWORK tags)
- Parser and lexer edge-case coverage (I've extensively tested these but more tests are welcome)
- A pretty printing module for displaying matrices to the terminal. This is the last module of the main program, which I have yet to start working on.
- Cross-platform testing and memory-safety improvements
- Documentation, examples, and CLI usability

Before opening a PR, please keep changes focused, add or update relevant tests, and run:

```sh
make test
```

If your change affects allocation, ownership, or error paths, please also run:

```sh
make memcheck
```

