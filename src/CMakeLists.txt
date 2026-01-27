# Core library (Phase 1 - in progress)
add_library(xasm_core STATIC
    core/atom.cpp
    core/expression.cpp
    core/symbol.cpp
    core/section.cpp
    core/assembler.cpp
    core/parse_utils.cpp
)

target_include_directories(xasm_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# CPU plugins library (Phase 1+)
add_library(xasm_cpu STATIC
    cpu/m6502/cpu_6502.cpp
    cpu/m6502/cpu_6502_branch_handler.cpp
    cpu/m6502/cpu_65c02.cpp
    cpu/m6502/cpu_65816.cpp
    # Future CPUs:
    # cpu/m6809/cpu_6809.cpp
    # cpu/m68k/cpu_68000.cpp
    # cpu/z80/cpu_z80.cpp
    # cpu/intel/cpu_8080.cpp
    # cpu/intel/cpu_8085.cpp
    # cpu/m6800/cpu_6800.cpp
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
    syntax/merlin/merlin_syntax.cpp
    # Future syntax plugins:
    # syntax/mos/mos_base_syntax.cpp
    # syntax/scmasm/scmasm_syntax.cpp
    # syntax/motorola/motorola_base_syntax.cpp
    # syntax/edtasm/edtasm_syntax.cpp
    # syntax/edtasm_m80/edtasm_m80_syntax.cpp
    # syntax/zilog/zilog_base_syntax.cpp
    # syntax/intel/intel_base_syntax.cpp
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
  # Future output plugins:
  # output/listing_output.cpp
  # output/symbol_output.cpp
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
