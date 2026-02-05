---
sidebar_position: 1
---

# Assembler API

The `Assembler` class is the main entry point for the xasm++ assembler engine.

## Overview

The assembler is responsible for:
- Managing sections (code, data)
- Coordinating multi-pass assembly
- Symbol resolution and label management
- CPU plugin integration
- Error reporting

## Basic Usage

```cpp
#include "assembler.h"
#include "cpu6502.h"

// Create assembler
Assembler assembler;

// Set CPU plugin
Cpu6502 cpu;
assembler.SetCpuPlugin(&cpu);

// Create and add section
Section section(".text", 
                static_cast<uint32_t>(SectionAttributes::Code), 
                0x8000);

// Add atoms (instructions, labels, data)
section.atoms.push_back(
    std::make_shared<InstructionAtom>("LDA", "#$42"));

// Add section to assembler
assembler.AddSection(section);

// Assemble
AssemblerResult result = assembler.Assemble();

// Check result
if (result.success) {
    // Success - access encoded bytes
} else {
    // Error - check result.errors
}
```

## Class: Assembler

### Constructor

```cpp
Assembler()
```

Creates a new assembler instance.

### Methods

#### SetCpuPlugin

```cpp
void SetCpuPlugin(CpuPlugin* plugin)
```

Sets the CPU plugin to use for instruction encoding.

**Parameters:**
- `plugin` - Pointer to CPU plugin implementation (e.g., `Cpu6502`)

#### AddSection

```cpp
void AddSection(const Section& section)
```

Adds a section to the assembler.

**Parameters:**
- `section` - Section containing atoms to assemble

#### Assemble

```cpp
AssemblerResult Assemble()
```

Performs multi-pass assembly of all sections.

**Returns:**
- `AssemblerResult` - Result object containing success status, errors, and warnings

**Process:**
1. Pass 1: Resolve symbols and calculate sizes
2. Pass N: Continue until convergence or max passes
3. Final: Encode all instructions

## Class: AssemblerResult

### Members

```cpp
struct AssemblerResult {
    bool success;                // True if assembly succeeded
    std::vector<std::string> errors;    // Error messages
    std::vector<std::string> warnings;  // Warning messages
};
```

## Example: Complete Program

```cpp
#include "assembler.h"
#include "cpu6502.h"
#include <iostream>

int main() {
    Assembler assembler;
    Cpu6502 cpu;
    assembler.SetCpuPlugin(&cpu);

    Section section(".text", 
                    static_cast<uint32_t>(SectionAttributes::Code), 
                    0x8000);

    // Simple loop program
    section.atoms.push_back(
        std::make_shared<InstructionAtom>("LDX", "#$00"));
    section.atoms.push_back(
        std::make_shared<LabelAtom>("loop", 0));
    section.atoms.push_back(
        std::make_shared<InstructionAtom>("INX", ""));
    section.atoms.push_back(
        std::make_shared<InstructionAtom>("CPX", "#$10"));
    section.atoms.push_back(
        std::make_shared<InstructionAtom>("BNE", "loop"));
    section.atoms.push_back(
        std::make_shared<InstructionAtom>("RTS", ""));

    assembler.AddSection(section);
    AssemblerResult result = assembler.Assemble();

    if (result.success) {
        std::cout << "Assembly successful!" << std::endl;
        
        // Output machine code
        for (auto& atom : section.atoms) {
            if (atom->type == AtomType::Instruction) {
                auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
                std::cout << inst->mnemonic << " ";
                for (uint8_t byte : inst->encoded_bytes) {
                    printf("%02X ", byte);
                }
                std::cout << std::endl;
            }
        }
    } else {
        std::cerr << "Assembly failed:" << std::endl;
        for (const auto& error : result.errors) {
            std::cerr << "  " << error << std::endl;
        }
        return 1;
    }

    return 0;
}
```

## See Also

- [Developer Documentation Overview](./overview.md) - API integration guide
- [Architecture Overview](../architecture/overview.md) - Plugin interfaces and system design
- [Quick Start Guide](../getting-started/quick-start.md) - Command-line usage (recommended for most users)
