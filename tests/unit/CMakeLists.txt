# Unit tests
# To add a test: xasm_add_unit_test(test_<name> LIBS <libraries>)
# xasm_add_unit_test is defined in tests/CMakeLists.txt.

# Core
xasm_add_unit_test(test_atom                    LIBS xasm_core)
xasm_add_unit_test(test_expression              LIBS xasm_core)
xasm_add_unit_test(test_symbol                  LIBS xasm_core)
xasm_add_unit_test(test_section                 LIBS xasm_core)
xasm_add_unit_test(test_assembler               LIBS xasm_core)
xasm_add_unit_test(test_parse_hex               LIBS xasm_core)
xasm_add_unit_test(test_address)
xasm_add_unit_test(test_error_formatter         LIBS xasm_core)
xasm_add_unit_test(test_conditional_assembler   LIBS xasm_core)
xasm_add_unit_test(test_current_location        LIBS xasm_core)
xasm_add_unit_test(test_expression_parser       LIBS xasm_core)
xasm_add_unit_test(test_radix_parsing           LIBS xasm_core)
xasm_add_unit_test(test_expression_utils        LIBS xasm_core)

# CPU plugins
xasm_add_unit_test(test_cpu6502                 LIBS xasm_cpu xasm_core
                                                EXTRA_SOURCES test_65816_extended.cpp)
xasm_add_unit_test(test_cpu_65c02               LIBS xasm_cpu xasm_core)
xasm_add_unit_test(test_cpu6809                 LIBS xasm_cpu xasm_core)
xasm_add_unit_test(test_cpu6809_indexed         LIBS xasm_cpu xasm_core)
xasm_add_unit_test(test_cpu_z80                 LIBS xasm_cpu xasm_core)
xasm_add_unit_test(test_cpu_error_utils         LIBS xasm_cpu xasm_core)
xasm_add_unit_test(test_mnemonic_constants      LIBS xasm_cpu xasm_core)
xasm_add_unit_test(test_opcodes_z80             LIBS xasm_cpu xasm_core)
xasm_add_unit_test(test_encoding_utils          INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)

# Syntax plugins
xasm_add_unit_test(test_simple_syntax           LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_merlin_syntax           LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_xc_directive            LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_scmasm_syntax           LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_scmasm_directive_registry LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_scmasm_conditionals     LIBS xasm_syntax xasm_core xasm_cpu)
xasm_add_unit_test(test_scmasm_expression_utils LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_edtasm_syntax           LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_edtasm_m80_plusplus_syntax LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_flex_syntax             LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_flex_macros             LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_flex_conditionals       LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_macro_processor         LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_core_directive_handlers LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_directive_registry      LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_directive_error_utils   LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_phase_directives        LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_segment_directives      LIBS xasm_syntax xasm_core
                                                INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
xasm_add_unit_test(test_inb_path_mapping        LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_z80_macros              LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_z80_directive_constants LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_z80_forward_refs        LIBS xasm_syntax xasm_cpu xasm_core)
xasm_add_unit_test(test_z80_expressions         LIBS xasm_syntax xasm_cpu xasm_core)

# Output plugins
xasm_add_unit_test(test_binary_output           LIBS xasm_output xasm_core)
xasm_add_unit_test(test_listing_output          LIBS xasm_output xasm_core)
xasm_add_unit_test(test_z80_listing_directives  LIBS xasm_output xasm_core)
xasm_add_unit_test(test_symbol_output           LIBS xasm_output xasm_core)
xasm_add_unit_test(test_intel_hex_writer        LIBS xasm_output xasm_core)
xasm_add_unit_test(test_srec_writer             LIBS xasm_output xasm_core)
xasm_add_unit_test(test_coco_loadm_writer       LIBS xasm_output xasm_core)
xasm_add_unit_test(test_trsdos_writer           LIBS xasm_output xasm_core)

# Cross-layer
xasm_add_unit_test(test_binary_compat           LIBS xasm_syntax xasm_output xasm_core)
xasm_add_unit_test(test_output_integration      LIBS xasm_output xasm_syntax xasm_cpu xasm_core
                                                INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
xasm_add_unit_test(test_expression_error_handling LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_exception_safety        LIBS xasm_core xasm_cpu)
xasm_add_unit_test(test_error_reporting         LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_const_correctness       LIBS xasm_core xasm_cpu)
xasm_add_unit_test(test_const_correctness_syntax LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_const_correctness_utility_methods LIBS xasm_syntax xasm_core)
xasm_add_unit_test(test_parser_error_utils      LIBS xasm_syntax xasm_core GTest::gmock)

# CLI / executable
xasm_add_unit_test(test_cli                     LIBS CLI11::CLI11
                                                EXTRA_SOURCES ../../src/cli_parser.cpp
                                                INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
