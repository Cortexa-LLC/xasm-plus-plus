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

# Future tests (Phase 1):
# add_executable(test_expression test_expression.cpp)
# add_executable(test_symbol test_symbol.cpp)
# add_executable(test_section test_section.cpp)
