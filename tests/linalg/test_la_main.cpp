/*
* Entry point to the linalg test suite.
*/

int main(void) {
    
    /*
    * We'll implement a structure for linalg that is similar 
    * to the lin tests. This main function will call upon run_..._tests
    * function that in turns calls upon many other functions that implement
    * unit tests. Each run_..._tests function will live inside of compilation
    * units that directly map to each major component of the linalg library.
    * In other words, we'll have something like test_arithmetic.*, 
    * test_row_operations.*, test_unary_operations.* and so on.
    *
    * We'll want the same fork-exec sandboxing used in lin.
    */

    return 0;
}
