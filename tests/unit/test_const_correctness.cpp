// Test const-correctness for non-modifying methods
// Addresses Minor Issue N1: Add const qualifiers to methods that don't modify object state

#include <gtest/gtest.h>
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include "xasm++/assembler.h"
#include "xasm++/atom.h"
#include "xasm++/section.h"

using namespace xasm;

// ============================================================================
// Test Helper: Verify const methods can be called on const objects
// ============================================================================

// Test CPU6502 const methods
TEST(ConstCorrectnessTest, Cpu6502GettersAreConst) {
    const Cpu6502 cpu;
    
    // These methods should be callable on const objects
    EXPECT_EQ("6502", cpu.GetName());
    EXPECT_EQ(CpuMode::Cpu6502, cpu.GetCpuMode());
    EXPECT_TRUE(cpu.IsAccumulator8Bit());  // Default is 8-bit
    EXPECT_TRUE(cpu.IsIndex8Bit());        // Default is 8-bit
}

// Test CPU6502 encoding methods are const
TEST(ConstCorrectnessTest, Cpu6502EncodingMethodsAreConst) {
    const Cpu6502 cpu;
    
    // All encoding methods should be const (don't modify CPU state)
    auto nop = cpu.EncodeNOP();
    EXPECT_EQ(1, nop.size());
    EXPECT_EQ(0xEA, nop[0]);
    
    auto rts = cpu.EncodeRTS();
    EXPECT_EQ(1, rts.size());
    EXPECT_EQ(0x60, rts[0]);
    
    auto lda = cpu.EncodeLDA(0x42, AddressingMode::Immediate);
    EXPECT_EQ(2, lda.size());
    EXPECT_EQ(0xA9, lda[0]);
    EXPECT_EQ(0x42, lda[1]);
}

// Test CPU6502 size calculation is const
TEST(ConstCorrectnessTest, Cpu6502CalculateInstructionSizeIsConst) {
    const Cpu6502 cpu;
    
    EXPECT_EQ(1, cpu.CalculateInstructionSize(AddressingMode::Implied));
    EXPECT_EQ(2, cpu.CalculateInstructionSize(AddressingMode::Immediate));
    EXPECT_EQ(3, cpu.CalculateInstructionSize(AddressingMode::Absolute));
}

// Test CPU6502 branch relaxation methods are const
TEST(ConstCorrectnessTest, Cpu6502BranchRelaxationIsConst) {
    const Cpu6502 cpu;
    
    // Branch relaxation methods should be const
    EXPECT_FALSE(cpu.NeedsBranchRelaxation(0x1000, 0x1010));  // Short branch
    EXPECT_TRUE(cpu.NeedsBranchRelaxation(0x1000, 0x1100));   // Long branch
}

// Test Expression getters are const
TEST(ConstCorrectnessTest, ExpressionGettersAreConst) {
    // LiteralExpr
    const LiteralExpr literal(42);
    EXPECT_EQ(42, literal.GetValue());
    EXPECT_TRUE(literal.IsConstant());
    EXPECT_FALSE(literal.IsRelocatable());
    
    // SymbolExpr
    const SymbolExpr symbol("label");
    EXPECT_EQ("label", symbol.GetSymbol());
    EXPECT_FALSE(symbol.IsConstant());
    EXPECT_TRUE(symbol.IsRelocatable());
    
    // BinaryOpExpr
    auto left = std::make_shared<LiteralExpr>(10);
    auto right = std::make_shared<LiteralExpr>(5);
    const BinaryOpExpr binary(BinaryOp::Add, left, right);
    EXPECT_EQ(BinaryOp::Add, binary.GetOperation());
    EXPECT_EQ(left, binary.GetLeft());
    EXPECT_EQ(right, binary.GetRight());
    EXPECT_TRUE(binary.IsConstant());
    EXPECT_FALSE(binary.IsRelocatable());
    
    // UnaryOpExpr
    auto operand = std::make_shared<LiteralExpr>(42);
    const UnaryOpExpr unary(UnaryOp::Negate, operand);
    EXPECT_EQ(UnaryOp::Negate, unary.GetOperation());
    EXPECT_EQ(operand, unary.GetOperand());
    EXPECT_TRUE(unary.IsConstant());
    EXPECT_FALSE(unary.IsRelocatable());
}

// Test SymbolTable const methods
TEST(ConstCorrectnessTest, SymbolTableLookupIsConst) {
    ConcreteSymbolTable symbols;
    symbols.Define("label", SymbolType::Label, std::make_shared<LiteralExpr>(0x1000));
    
    // Cast to const to verify Lookup is const
    const ConcreteSymbolTable& const_symbols = symbols;
    
    int64_t value;
    EXPECT_TRUE(const_symbols.Lookup("label", value));
    EXPECT_EQ(0x1000, value);
    
    EXPECT_FALSE(const_symbols.Lookup("undefined", value));
}

// Test SymbolTable GetSymbol const variant
TEST(ConstCorrectnessTest, SymbolTableGetSymbolConstIsConst) {
    ConcreteSymbolTable symbols;
    symbols.Define("label", SymbolType::Label, std::make_shared<LiteralExpr>(0x2000));
    
    // Test const variant
    const ConcreteSymbolTable& const_symbols = symbols;
    const Symbol* sym = const_symbols.GetSymbol("label");
    ASSERT_NE(nullptr, sym);
    EXPECT_EQ("label", sym->name);
    
    // Non-existent symbol
    EXPECT_EQ(nullptr, const_symbols.GetSymbol("undefined"));
}

// Test SymbolTable IsDefined is const
TEST(ConstCorrectnessTest, SymbolTableIsDefinedIsConst) {
    ConcreteSymbolTable symbols;
    symbols.Define("test", SymbolType::Label, std::make_shared<LiteralExpr>(0x3000));
    
    const ConcreteSymbolTable& const_symbols = symbols;
    EXPECT_TRUE(const_symbols.IsDefined("test"));
    EXPECT_FALSE(const_symbols.IsDefined("missing"));
}

// Test SymbolTable GetAllSymbols is const
TEST(ConstCorrectnessTest, SymbolTableGetAllSymbolsIsConst) {
    ConcreteSymbolTable symbols;
    symbols.Define("sym1", SymbolType::Label, std::make_shared<LiteralExpr>(0x100));
    symbols.Define("sym2", SymbolType::Equate, std::make_shared<LiteralExpr>(0x200));
    
    const ConcreteSymbolTable& const_symbols = symbols;
    const auto& all_symbols = const_symbols.GetAllSymbols();
    EXPECT_EQ(2, all_symbols.size());
    EXPECT_TRUE(all_symbols.find("sym1") != all_symbols.end());
    EXPECT_TRUE(all_symbols.find("sym2") != all_symbols.end());
}

// Test Assembler GetSectionCount is const
TEST(ConstCorrectnessTest, AssemblerGetSectionCountIsConst) {
    Assembler assembler;
    
    Section section1("code", 0, 0x1000);
    Section section2("data", 0, 0x2000);
    
    assembler.AddSection(section1);
    assembler.AddSection(section2);
    
    // Cast to const to verify GetSectionCount is const
    const Assembler& const_assembler = assembler;
    EXPECT_EQ(2, const_assembler.GetSectionCount());
}

// Test that const objects can use const methods in realistic scenarios
TEST(ConstCorrectnessTest, ConstMethodsWorkInRealWorldScenario) {
    // Create const CPU
    Cpu6502 mutable_cpu;
    mutable_cpu.SetCpuMode(CpuMode::Cpu65C02);
    const Cpu6502& cpu = mutable_cpu;
    
    // All these should work on const reference
    EXPECT_EQ(CpuMode::Cpu65C02, cpu.GetCpuMode());
    auto lda = cpu.EncodeLDA(0xFF, AddressingMode::Immediate);
    EXPECT_FALSE(lda.empty());
    
    // Create const symbol table
    ConcreteSymbolTable mutable_symbols;
    mutable_symbols.Define("start", SymbolType::Label, std::make_shared<LiteralExpr>(0x8000));
    const ConcreteSymbolTable& symbols = mutable_symbols;
    
    int64_t addr;
    EXPECT_TRUE(symbols.Lookup("start", addr));
    EXPECT_EQ(0x8000, addr);
    
    // Create const expression
    auto expr = std::make_shared<LiteralExpr>(100);
    const Expression& const_expr = *expr;
    EXPECT_TRUE(const_expr.IsConstant());
    EXPECT_EQ(100, const_expr.Evaluate(symbols));
}
