# Core library (Phase 1 - in progress)
add_library(xasm_core STATIC
    core/atom.cpp
    core/expression.cpp
    core/symbol.cpp
    core/section.cpp
    core/segment_manager.cpp
    core/assembler.cpp
    core/parse_utils.cpp
    core/error_formatter.cpp
    utils/string_utils.cpp
    common/expression_parser.cpp
    common/macro_processor.cpp
    common/conditional_assembler.cpp
)

target_include_directories(xasm_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# CPU plugins library (Phase 1+)
add_library(xasm_cpu STATIC
    cpu/cpu_6502.cpp
    cpu/cpu_6502_branch_handler.cpp
    cpu/cpu_65c02.cpp
    cpu/cpu_65816.cpp
    cpu/cpu_6809.cpp
    cpu/cpu_z80.cpp
    # Future CPUs:
    # cpu/cpu_68000.cpp
    # cpu/cpu_8080.cpp
    # cpu/cpu_8085.cpp
    # cpu/cpu_6800.cpp
)

target_include_directories(xasm_cpu PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link core with CPU plugin
target_link_libraries(xasm_core PUBLIC
    xasm_cpu
)

# Syntax plugins library (Phase 1+)
add_library(xasm_syntax STATIC
    syntax/simple_syntax.cpp
    syntax/merlin_syntax.cpp
    syntax/merlin_directives.cpp
    syntax/scmasm_syntax.cpp
    syntax/scmasm_directive_handlers.cpp
    syntax/edtasm_syntax.cpp
    syntax/flex_syntax.cpp
    syntax/directive_registry.cpp
    syntax/edtasm_m80_plusplus_syntax.cpp
    syntax/edtasm_directive_handlers.cpp
    syntax/core_directive_handlers.cpp
    # Future syntax plugins:
    # syntax/mos_base_syntax.cpp
    # syntax/motorola_base_syntax.cpp
    # syntax/zilog_base_syntax.cpp
    # syntax/intel_base_syntax.cpp
)

target_include_directories(xasm_syntax PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(xasm_syntax PUBLIC
    xasm_core
)

# Output plugins library (Phase 1+)
add_library(xasm_output STATIC
  output/binary_output.cpp
  output/listing_output.cpp
  output/symbol_output.cpp
  output/output_writer.cpp
  output/intel_hex_writer.cpp
  # Future output plugins:
  # output/hex_output.cpp
  # output/apple2/dos33_output.cpp
  # output/apple2/prodos_output.cpp
  # output/apple2gs/omf_output.cpp
  # output/coco/decb_output.cpp
  # output/coco/os9_output.cpp
  # output/trs80/cmd_output.cpp
  # output/cpm/com_output.cpp
)

target_include_directories(xasm_output PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(xasm_output PUBLIC
  xasm_core
)

# Main executable (Phase 1+)
add_executable(xasm++
    main.cpp
    cli_parser.cpp
)

target_link_libraries(xasm++
    xasm_core
    xasm_cpu
    xasm_syntax
    xasm_output
    CLI11::CLI11
)
