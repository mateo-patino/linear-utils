/*
* Entry point to the linalg test suite.
*/
#include "linalg/test_arithmetic.hpp"
#include "linalg/test_la_helpers.hpp"

#include <iomanip>
#include <iostream>
#include <string_view>


constexpr std::string_view LINE = "------------";


static void print_overall_summary(unsigned total, unsigned pass) {
    std::cout << BOLD << LINE << "SUMMARY" << LINE << ANSI_RESET << '\n';
    std::cout << BOLD << "Total tests: " << total << ANSI_RESET << '\n';
    std::cout << BOLD << "Successful: " << pass << ANSI_RESET << '\n';
    std::cout << BOLD << "Failed: " << total - pass << ANSI_RESET << '\n';

    const double success_rate = total == 0 ? 0.0 : 100.0 * pass / total;
    std::cout << BOLD << ANSI_CYAN << "Overall success " << std::fixed << std::setprecision(0) << success_rate
              << "%" << ANSI_RESET << '\n';
}


static void print_test_summary(std::string_view kind, unsigned total, unsigned pass) {
    std::cout << BOLD << kind << " Results: " << total << " total, " << pass << " pass, " << total - pass
              << " fail" << ANSI_RESET << '\n';
}

int main(void) {
    
    std::cout << BOLD << "\nLINEAR ALGEBRA LIBRARY TEST SUITE" << ANSI_RESET << '\n';
    std::cout << "Note that this program does not test the main `lin` app, only the linear algebra routines.\n"
              << "Run `make test` instead to run the `lin` test suite.\n";

    unsigned total_tests = 0;
    unsigned total_pass = 0;

    std::cout << '\n';
    std::cout << BOLD << LINE << "ARITHMETIC TESTS" << LINE << ANSI_RESET << '\n';

    int arithmetic_total = 0;
    unsigned arithmetic_success_count = run_arithmetic_tests(arithmetic_total);
    print_test_summary("Arithmetic", arithmetic_total, arithmetic_success_count);

    total_tests += arithmetic_total;
    total_pass += arithmetic_success_count;

    std::cout << '\n';
    print_overall_summary(total_tests, total_pass);

    return 0;
}
