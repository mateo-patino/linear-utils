*******************************************************
This module is responsible for printing results to the terminal.
A result may consist of a scalars, matrices, and `lin` language syntax.

*******************************************************


KEY DESIGN CHARACTERISTICS:

- The module shall be able to pretty print scalars and matrices. Users shall be given
  a flag they can toggle to have matrices printer in `lin`-like format (e.g. [[1, 2, 3, 4]]
  gets printed as 2x2 1 2 3 4).  

- The module defines a printout_t struct which wraps a pointer to an 
  object to be printed. This is the main struct on which the functions in
  this module will operate.

- The module uses the type interface defined src/types/. In particular, printout_t
  structs will point to scalar_t objects for scalars, matrix_t objects for matrices,
  and char * for `lin` syntax strings.

