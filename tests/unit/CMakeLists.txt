# Unit tests

# Atom tests
add_executable(test_atom
    test_atom.cpp
)

target_link_libraries(test_atom
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_atom)

# Expression tests
add_executable(test_expression
    test_expression.cpp
)

target_link_libraries(test_expression
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_expression)

# Symbol tests
add_executable(test_symbol
    test_symbol.cpp
)

target_link_libraries(test_symbol
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_symbol)

# Section tests
add_executable(test_section
    test_section.cpp
)

target_link_libraries(test_section
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_section)

# Future tests (Phase 1):
