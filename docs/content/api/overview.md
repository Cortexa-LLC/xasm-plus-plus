---
sidebar_position: 0
---

# Developer Documentation

This section contains API documentation for developers who want to integrate xasm++ into their own tools and applications.

## Who Is This For?

The API documentation is intended for developers who want to:
- Embed the xasm++ assembler engine in their applications
- Create custom build tools that use xasm++ programmatically
- Develop IDE plugins or language servers
- Extend xasm++ with new CPU plugins or syntax modes

**Note:** Most users should use the [command-line interface](../getting-started/quick-start.md) instead.

## API Overview

### Core Components

**[Assembler](./assembler.md)** - Main assembler engine
- Multi-pass assembly coordination
- Section and symbol management
- Error reporting

**CPU Plugins** - CPU instruction encoding
- `CpuPlugin` base interface
- `Cpu6502` - 6502 family implementation
- `Cpu6809` - Motorola 6809 implementation

**Syntax Parsers** - Source code parsing
- `MerlinSyntax` - Merlin assembler syntax
- `ScmasmSyntax` - S-C Macro Assembler syntax
- `EdtasmSyntax` - EDTASM syntax

### Basic Integration

```cpp
#include "assembler.h"
#include "cpu6502.h"
#include "merlin_syntax.h"

// Create assembler with 6502 CPU
Assembler assembler;
Cpu6502 cpu;
assembler.SetCpuPlugin(&cpu);

// Parse source file
MerlinSyntax parser;
auto sections = parser.ParseFile("source.asm", &cpu);

// Assemble
for (const auto& section : sections) {
    assembler.AddSection(section);
}

AssemblerResult result = assembler.Assemble();

if (result.success) {
    // Generate output
    BinaryOutput output;
    output.Write("output.bin", assembler.GetSections());
}
```

## Plugin Development

### Creating a CPU Plugin

To add support for a new CPU:

1. Implement the `CpuPlugin` interface:
```cpp
class CpuZ80 : public CpuPlugin {
public:
    std::vector<uint8_t> EncodeInstruction(
        const std::string& mnemonic,
        uint32_t operand,
        const std::string& operand_str
    ) const override;

    // ... other interface methods
};
```

2. Register the plugin:
```cpp
CpuRegistry::Instance().Register("z80",
    []() { return std::make_unique<CpuZ80>(); });
```

### Creating a Syntax Parser

To add support for a new syntax mode:

1. Implement the `SyntaxPlugin` interface
2. Register with `SyntaxRegistry`
3. Follow existing patterns from Merlin/SCMASM/EDTASM

See the architecture documentation for detailed plugin interfaces.

## API Stability

**Current Status:** The core API is stable and suitable for production use.

**Versioning:** xasm++ follows semantic versioning:
- Major version changes may break API compatibility
- Minor version changes add features while maintaining compatibility
- Patch versions fix bugs without changing the API

## Build Integration

### CMake Integration

```cmake
# Add xasm++ as a subdirectory
add_subdirectory(xasm++)

# Link against xasm++ libraries
target_link_libraries(your_app PRIVATE
    xasm_core
    cpu_plugins
    syntax_plugins
)
```

### Header-Only Usage

For simple integration, include only what you need:
```cpp
#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_6502.h"
```

## Examples

See the [Assembler API](./assembler.md) page for complete code examples.

## Further Reading

- [Architecture Overview](../architecture/overview.md) - System design
- [Quick Start Guide](../getting-started/quick-start.md) - CLI usage
- [Reference Documentation](../reference/merlin-syntax-reference.md) - Syntax details
