/**
 * @file atom.h
 * @brief Intermediate Representation (IR) for assembly language constructs
 *
 * This file defines the atom system, which represents assembly language
 * constructs as a polymorphic hierarchy of objects. Atoms form the internal
 * representation used during multi-pass assembly.
 *
 * @note Inspired by vasm-ext atom chain design
 * @note Phase 1: Minimal Viable Assembler - Core Abstractions
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace xasm {

/**
 * @brief Types of atoms in the intermediate representation
 *
 * Each atom type represents a different assembly language construct.
 */
enum class AtomType {
  Label,          ///< Symbol definition (e.g., "start:" or "loop:")
  Instruction,    ///< CPU instruction (e.g., "LDA #$42")
  Data,           ///< Raw data bytes (e.g., ".byte $01, $02, $03")
  Space,          ///< Reserved/uninitialized space (e.g., ".ds 100")
  Align,          ///< Alignment directive (e.g., ".align 256")
  Org,            ///< Origin directive (e.g., ".org $8000")
  ListingControl, ///< Listing control directive (e.g., "TITLE", "PAGE", "LIST")
};

/**
 * @brief Source code location for error reporting
 *
 * Tracks the file, line, and column where an atom originated in the
 * source code. Used for generating helpful error messages.
 */
struct SourceLocation {
  std::string filename; ///< Source filename
  int line;             ///< Line number (1-based)
  int column;           ///< Column number (1-based)

  /** @brief Default constructor - creates invalid location */
  SourceLocation() : line(0), column(0) {}

  /**
   * @brief Construct with file, line, and column
   * @param file Source filename
   * @param l Line number
   * @param c Column number
   */
  SourceLocation(const std::string &file, int l, int c)
      : filename(file), line(l), column(c) {}
};

/**
 * @brief Base class for all atoms in the intermediate representation
 *
 * An atom represents a single construct in assembly language (label,
 * instruction, data, etc.). Atoms are organized in a linked list and processed
 * during multi-pass assembly. The size of an atom may change between passes as
 * symbols are resolved and branch relaxation occurs.
 *
 * @par Size Tracking
 * Each atom tracks its size in bytes. During multi-pass assembly:
 * - `size` is the current size after this pass
 * - `last_size` is the size from the previous pass
 * - `changes` counts how many times the size has changed
 *
 * @par Polymorphism
 * Atom is an abstract base class. Concrete atom types derive from this
 * class and provide additional type-specific data (e.g., InstructionAtom
 * stores mnemonic and operand).
 */
class Atom {
public:
  AtomType type;              ///< Type of this atom
  SourceLocation location;    ///< Source location for error reporting
  size_t size;                ///< Size in bytes (current pass)
  size_t last_size;           ///< Size in bytes (previous pass)
  uint32_t changes;           ///< Number of times size has changed
  std::shared_ptr<Atom> next; ///< Next atom in linked list (nullptr if last)
  std::string source_line;    ///< Original source line text (for listing output)

  /**
   * @brief Construct an atom of the given type
   * @param t Atom type
   */
  explicit Atom(AtomType t)
      : type(t), size(0), last_size(0), changes(0), next(nullptr) {}

  /**
   * @brief Virtual destructor for polymorphic deletion
   */
  virtual ~Atom() = default;
};

/**
 * @brief Label atom - represents a symbol definition
 *
 * A label defines a symbol at the current program counter. Labels do not
 * generate any bytes in the output but serve as reference points for
 * other instructions and data.
 *
 * @par Example
 * @code
 * start:        ; LabelAtom(name="start", address=0x8000)
 *   LDA #$42
 * @endcode
 */
class LabelAtom : public Atom {
public:
  std::string name; ///< Symbol name
  uint32_t address; ///< Resolved address of this label

  /**
   * @brief Construct a label atom
   * @param n Symbol name
   * @param addr Address where this label is defined
   */
  LabelAtom(const std::string &n, uint32_t addr)
      : Atom(AtomType::Label), name(n), address(addr) {
    size = 0; // Labels don't take space
  }
};

/**
 * @brief Instruction atom - represents a CPU instruction
 *
 * An instruction atom represents a single CPU instruction with its
 * mnemonic and operand. The instruction is encoded into machine code
 * by the CPU plugin during assembly.
 *
 * @par Example
 * @code
 * LDA #$42      ; InstructionAtom(mnemonic="LDA", operand="#$42")
 * @endcode
 *
 * @note The encoded_bytes field is populated by the CPU plugin during
 *       the encoding phase of assembly
 */
class InstructionAtom : public Atom {
public:
  std::string mnemonic; ///< Instruction mnemonic (e.g., "LDA", "STA")
  std::string operand;  ///< Operand string (e.g., "#$42", "$1234,X")
  std::vector<uint8_t>
      encoded_bytes; ///< Encoded machine code (set by CPU plugin)

  /**
   * @brief Construct an instruction atom
   * @param mnem Instruction mnemonic
   * @param oper Operand string
   */
  InstructionAtom(const std::string &mnem, const std::string &oper)
      : Atom(AtomType::Instruction), mnemonic(mnem), operand(oper) {
    // Size determined during encoding phase
  }
};

/**
 * @brief Data size for data atoms (byte vs word)
 */
enum class DataSize {
  Byte, ///< 8-bit data (db, dfb directives)
  Word  ///< 16-bit data (dw, da directives)
};

/**
 * @brief Data atom - represents raw data bytes or expressions
 *
 * A data atom contains raw bytes or expressions that will be evaluated
 * and emitted to the output. This is used for directives like .byte,
 * .word, .db, .dw, etc.
 *
 * The atom stores both the original expression strings and the evaluated
 * bytes. The expressions are re-evaluated on each pass to resolve forward
 * references and ensure correct addresses.
 *
 * @par Example
 * @code
 * .byte $01, $02, $03   ; DataAtom(expressions={"$01", "$02", "$03"},
 * data_size=Byte) .word start, end      ; DataAtom(expressions={"start",
 * "end"}, data_size=Word)
 * @endcode
 */
class DataAtom : public Atom {
public:
  std::vector<std::string> expressions; ///< Original expression strings
  std::vector<uint8_t> data;            ///< Evaluated data bytes
  DataSize data_size;                   ///< Size of each data element

  /**
   * @brief Construct a data atom with expressions (for multi-pass evaluation)
   * @param exprs Vector of expression strings
   * @param size Data size (Byte or Word)
   */
  DataAtom(const std::vector<std::string> &exprs, DataSize size)
      : Atom(AtomType::Data), expressions(exprs), data_size(size) {
    // Size will be determined after evaluation
    this->size = 0;
  }

  /**
   * @brief Construct a data atom with pre-evaluated bytes (legacy constructor)
   * @param d Vector of bytes to emit
   */
  explicit DataAtom(const std::vector<uint8_t> &d)
      : Atom(AtomType::Data), data(d), data_size(DataSize::Byte) {
    size = data.size();
  }
};

/**
 * @brief Space atom - represents reserved/uninitialized space
 *
 * A space atom reserves a specified number of bytes without initializing
 * them. This is used for directives like .ds (define space) or .res.
 *
 * @par Example
 * @code
 * .ds 100       ; SpaceAtom(count=100) - reserve 100 bytes
 * @endcode
 */
class SpaceAtom : public Atom {
public:
  size_t count; ///< Number of bytes to reserve

  /**
   * @brief Construct a space atom
   * @param c Number of bytes to reserve
   */
  explicit SpaceAtom(size_t c) : Atom(AtomType::Space), count(c) {
    size = count;
  }
};

/**
 * @brief Align atom - represents an alignment directive
 *
 * An align atom pads the current address to the next multiple of the
 * specified alignment. The actual number of padding bytes is determined
 * during the layout phase based on the current address.
 *
 * @par Example
 * @code
 * .align 256    ; AlignAtom(alignment=256) - pad to next 256-byte boundary
 * @endcode
 */
class AlignAtom : public Atom {
public:
  size_t alignment; ///< Alignment boundary (must be power of 2)

  /**
   * @brief Construct an align atom
   * @param align Alignment boundary in bytes
   */
  explicit AlignAtom(size_t align) : Atom(AtomType::Align), alignment(align) {
    // Size determined during layout phase
  }
};

/**
 * @brief Org atom - represents an origin directive
 *
 * An org atom sets the program counter to a specific address. It does not
 * generate any bytes but affects the addresses of subsequent atoms.
 *
 * @par Example
 * @code
 * .org $8000    ; OrgAtom(address=0x8000) - set PC to 0x8000
 * @endcode
 */
class OrgAtom : public Atom {
public:
  uint32_t address; ///< New program counter address

  /**
   * @brief Construct an org atom
   * @param addr Address to set the program counter to
   */
  explicit OrgAtom(uint32_t addr) : Atom(AtomType::Org), address(addr) {
    size = 0; // ORG doesn't generate bytes
  }
};

/**
 * @brief Listing control type for directives
 */
enum class ListingControlType {
  Title,    ///< TITLE directive - sets page title
  Subtitle, ///< SUBTTL directive - sets page subtitle
  Page,     ///< PAGE/EJECT directive - forces page break
  Space,    ///< SPACE directive - inserts blank lines
  List,     ///< LIST directive - enables listing output
  Nolist,   ///< NOLIST directive - disables listing output
  Lall,     ///< LALL directive - list all macro expansions
  Sall,     ///< SALL directive - suppress macro expansion listing
};

/**
 * @brief Listing control atom - represents listing directives
 *
 * A listing control atom controls how the listing file is formatted and
 * what content is included. These directives affect listing output only
 * and do not generate any bytes in the binary.
 *
 * @par Example
 * @code
 * TITLE "My Program"    ; ListingControlAtom(type=Title, value="My Program")
 * PAGE                  ; ListingControlAtom(type=Page)
 * SPACE 3               ; ListingControlAtom(type=Space, count=3)
 * LIST                  ; ListingControlAtom(type=List)
 * NOLIST                ; ListingControlAtom(type=Nolist)
 * @endcode
 */
class ListingControlAtom : public Atom {
public:
  ListingControlType control_type; ///< Type of listing control
  std::string value;               ///< String value (for TITLE)
  int count;                       ///< Numeric value (for SPACE)

  /**
   * @brief Construct a listing control atom with string value
   * @param type Control type
   * @param val String value
   */
  ListingControlAtom(ListingControlType type, const std::string &val)
      : Atom(AtomType::ListingControl), control_type(type), value(val),
        count(0) {
    size = 0; // Listing control doesn't generate bytes
  }

  /**
   * @brief Construct a listing control atom with numeric value
   * @param type Control type
   * @param cnt Numeric value
   */
  ListingControlAtom(ListingControlType type, int cnt)
      : Atom(AtomType::ListingControl), control_type(type), count(cnt) {
    size = 0; // Listing control doesn't generate bytes
  }

  /**
   * @brief Construct a listing control atom with no value
   * @param type Control type
   */
  explicit ListingControlAtom(ListingControlType type)
      : Atom(AtomType::ListingControl), control_type(type), count(0) {
    size = 0; // Listing control doesn't generate bytes
  }
};

} // namespace xasm
