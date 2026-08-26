*******************************************************
This a small linear algebra library.

'lin' calls this library to perform all linear algebra math.
*******************************************************


KEY DESIGN CHARACTERISTICS:

- The library is completely agnostic of `lin`. The library has its own data structures
  and types that do not rely on `lin` whatsoever.

- The library reads data from addresses provided by a caller and writes data to addresses
  provided by the caller. The library does not allocate or free any memory. 

- The library uses 2-strided views to read and write matrix data, defined via `matrixv_t`
  in `view.h`. Currently we'll only implement naive operations without any striding 
  supported, but eventually we'll move on past the naive approaches, as some algorithms
  work on submatrix views rather than .
