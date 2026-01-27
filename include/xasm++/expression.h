/**
 * @file expression.h
 * @brief Expression trees for complex expressions and forward references
 * 
 * This file defines a polymorphic expression tree system that supports
 * arithmetic and logical operations on constants and symbol references.
 * Expressions are evaluated during assembly to resolve operand values.
 * 
 * @note Phase 1: Minimal Viable Assembler - Core Abstractions
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <map>

namespace xasm {

// Forward declaration
class SymbolTable;

/**
 * @brief Binary operators supported in expressions
 * 
 * These operators combine two sub-expressions to produce a result.
 * More operators will be added in future phases.
 */
enum class BinaryOp {
    Add,        ///< Addition: a + b
    Subtract,   ///< Subtraction: a - b
    Multiply,   ///< Multiplication: a * b
    Divide,     ///< Division: a / b
    // More operators to be added in future phases:
    // Modulo, BitwiseAnd, BitwiseOr, BitwiseXor, ShiftLeft, ShiftRight, etc.
};

/**
 * @brief Unary operators supported in expressions
 * 
 * These operators apply to a single sub-expression to produce a result.
 * More operators will be added in future phases.
 */
enum class UnaryOp {
    Negate,     ///< Negation: -a
    BitwiseNot, ///< Bitwise NOT: ~a
    // More operators to be added in future phases:
    // LogicalNot, etc.
};

/**
 * @brief Abstract base class for all expressions
 * 
 * An expression represents a value that can be computed from constants
 * and symbol references. Expressions form a tree structure where:
 * - Leaf nodes are literals or symbol references
 * - Internal nodes are operators (binary or unary)
 * 
 * @par Expression Evaluation
 * Expressions are evaluated during assembly passes. If an expression
 * references undefined symbols, evaluation will throw an exception.
 * 
 * @par Example Expression Trees
 * @code
 * // "42" -> LiteralExpr(42)
 * // "start" -> SymbolExpr("start")
 * // "start + 2" -> BinaryOpExpr(Add, SymbolExpr("start"), LiteralExpr(2))
 * // "-(end - start)" -> UnaryOpExpr(Negate, BinaryOpExpr(Subtract, ...))
 * @endcode
 */
class Expression {
public:
    /**
     * @brief Virtual destructor for polymorphic deletion
     */
    virtual ~Expression() = default;

    /**
     * @brief Evaluate the expression to a numeric value
     * 
     * Recursively evaluates the expression tree using the provided
     * symbol table to resolve symbol references.
     * 
     * @param symbols Symbol table for resolving symbol references
     * @return Computed numeric value of the expression
     * @throws std::runtime_error if expression references undefined symbols
     *         or performs invalid operations (e.g., division by zero)
     */
    virtual int64_t Evaluate(const SymbolTable& symbols) const = 0;

    /**
     * @brief Check if the expression is constant
     * 
     * A constant expression contains no symbol references and always
     * evaluates to the same value regardless of symbol table contents.
     * 
     * @return true if expression is constant, false if it contains symbols
     */
    virtual bool IsConstant() const = 0;

    /**
     * @brief Check if the expression is relocatable
     * 
     * A relocatable expression contains address symbols and may need
     * relocation when the program is loaded at a different address.
     * 
     * @return true if expression contains relocatable symbols, false otherwise
     */
    virtual bool IsRelocatable() const = 0;
};

/**
 * @brief Abstract interface for symbol lookup
 * 
 * The SymbolTable interface provides a way for expressions to look up
 * symbol values without depending on a concrete symbol table implementation.
 * This allows expressions to be evaluated in different contexts (e.g.,
 * during assembly, in test code, etc.).
 */
class SymbolTable {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~SymbolTable() = default;

    /**
     * @brief Look up a symbol by name
     * 
     * @param name Symbol name to look up
     * @param value Output parameter - receives the symbol's value if found
     * @return true if symbol was found, false otherwise
     */
    virtual bool Lookup(const std::string& name, int64_t& value) const = 0;
};

/**
 * @brief Literal expression - represents a constant value
 * 
 * A literal expression evaluates to a fixed numeric constant.
 * It is always constant and never relocatable.
 * 
 * @par Example
 * @code
 * LDA #42       ; Operand "42" -> LiteralExpr(42)
 * JMP $8000     ; Operand "$8000" -> LiteralExpr(0x8000)
 * @endcode
 */
class LiteralExpr : public Expression {
public:
    /**
     * @brief Construct a literal expression
     * @param val Constant numeric value
     */
    explicit LiteralExpr(int64_t val) : value(val) {}

    /**
     * @brief Evaluate - returns the constant value
     * @param symbols Symbol table (unused)
     * @return The constant value
     */
    int64_t Evaluate(const SymbolTable& /*symbols*/) const override {
        return value;
    }

    /**
     * @brief Check if constant - always returns true
     * @return true
     */
    bool IsConstant() const override {
        return true;
    }

    /**
     * @brief Check if relocatable - always returns false
     * @return false
     */
    bool IsRelocatable() const override {
        return false;
    }

    /**
     * @brief Get the constant value
     * @return The literal value
     */
    int64_t GetValue() const { return value; }

private:
    int64_t value;      ///< The constant value
};

/**
 * @brief Symbol expression - represents a symbol reference
 * 
 * A symbol expression evaluates to the value of a named symbol.
 * It is never constant and typically relocatable (unless it's
 * an absolute symbol like an EQU constant).
 * 
 * @par Example
 * @code
 * JMP start     ; Operand "start" -> SymbolExpr("start")
 * LDA end+1     ; First operand "end" -> SymbolExpr("end")
 * @endcode
 */
class SymbolExpr : public Expression {
public:
    /**
     * @brief Construct a symbol expression
     * @param sym Symbol name to reference
     */
    explicit SymbolExpr(const std::string& sym) : symbol(sym) {}

    /**
     * @brief Evaluate - looks up the symbol in the symbol table
     * 
     * @param symbols Symbol table for lookup
     * @return The symbol's current value
     * @throws std::runtime_error if symbol is not defined
     */
    int64_t Evaluate(const SymbolTable& symbols) const override {
        int64_t value = 0;
        if (!symbols.Lookup(symbol, value)) {
            throw std::runtime_error("Undefined symbol: " + symbol);
        }
        return value;
    }

    /**
     * @brief Check if constant - always returns false
     * @return false (symbol references are never constant)
     */
    bool IsConstant() const override {
        return false;  // Symbol reference is not constant
    }

    /**
     * @brief Check if relocatable - always returns true
     * @return true (symbol references are typically relocatable)
     */
    bool IsRelocatable() const override {
        return true;  // Symbol references are relocatable
    }

    /**
     * @brief Get the symbol name
     * @return The referenced symbol name
     */
    const std::string& GetSymbol() const { return symbol; }

private:
    std::string symbol;     ///< The symbol name
};

/**
 * @brief Binary operation expression - combines two expressions
 * 
 * A binary operation expression applies an operator (add, subtract, etc.)
 * to two sub-expressions. The result is constant only if both operands
 * are constant. The result is relocatable if either operand is relocatable.
 * 
 * @par Example
 * @code
 * start + 2     ; BinaryOpExpr(Add, SymbolExpr("start"), LiteralExpr(2))
 * end - start   ; BinaryOpExpr(Subtract, SymbolExpr("end"), SymbolExpr("start"))
 * 5 * 3         ; BinaryOpExpr(Multiply, LiteralExpr(5), LiteralExpr(3))
 * @endcode
 */
class BinaryOpExpr : public Expression {
public:
    /**
     * @brief Construct a binary operation expression
     * 
     * @param op Binary operator to apply
     * @param l Left operand expression
     * @param r Right operand expression
     */
    BinaryOpExpr(BinaryOp op, std::shared_ptr<Expression> l, std::shared_ptr<Expression> r)
        : operation(op), left(l), right(r) {}

    /**
     * @brief Evaluate the binary operation
     * 
     * Evaluates both operands and applies the operator.
     * 
     * @param symbols Symbol table for operand evaluation
     * @return Result of applying the operator to the operands
     * @throws std::runtime_error if division by zero or unknown operator
     */
    int64_t Evaluate(const SymbolTable& symbols) const override {
        int64_t lval = left->Evaluate(symbols);
        int64_t rval = right->Evaluate(symbols);

        switch (operation) {
            case BinaryOp::Add:
                return lval + rval;
            case BinaryOp::Subtract:
                return lval - rval;
            case BinaryOp::Multiply:
                return lval * rval;
            case BinaryOp::Divide:
                if (rval == 0) {
                    throw std::runtime_error("Division by zero");
                }
                return lval / rval;
            default:
                throw std::runtime_error("Unknown binary operator");
        }
    }

    /**
     * @brief Check if constant
     * @return true only if both operands are constant
     */
    bool IsConstant() const override {
        return left->IsConstant() && right->IsConstant();
    }

    /**
     * @brief Check if relocatable
     * @return true if either operand is relocatable
     */
    bool IsRelocatable() const override {
        return left->IsRelocatable() || right->IsRelocatable();
    }

    /**
     * @brief Get the operator
     * @return The binary operator
     */
    BinaryOp GetOperation() const { return operation; }
    
    /**
     * @brief Get the left operand
     * @return Shared pointer to left operand expression
     */
    const std::shared_ptr<Expression>& GetLeft() const { return left; }
    
    /**
     * @brief Get the right operand
     * @return Shared pointer to right operand expression
     */
    const std::shared_ptr<Expression>& GetRight() const { return right; }

private:
    BinaryOp operation;                 ///< The binary operator
    std::shared_ptr<Expression> left;   ///< Left operand
    std::shared_ptr<Expression> right;  ///< Right operand
};

/**
 * @brief Unary operation expression - applies an operator to one expression
 * 
 * A unary operation expression applies an operator (negate, bitwise NOT)
 * to a single sub-expression. The result inherits the constant and
 * relocatable properties of its operand.
 * 
 * @par Example
 * @code
 * -5            ; UnaryOpExpr(Negate, LiteralExpr(5))
 * ~$FF          ; UnaryOpExpr(BitwiseNot, LiteralExpr(0xFF))
 * -(end-start)  ; UnaryOpExpr(Negate, BinaryOpExpr(...))
 * @endcode
 */
class UnaryOpExpr : public Expression {
public:
    /**
     * @brief Construct a unary operation expression
     * 
     * @param op Unary operator to apply
     * @param operand Operand expression
     */
    UnaryOpExpr(UnaryOp op, std::shared_ptr<Expression> operand)
        : operation(op), expr(operand) {}

    /**
     * @brief Evaluate the unary operation
     * 
     * Evaluates the operand and applies the operator.
     * 
     * @param symbols Symbol table for operand evaluation
     * @return Result of applying the operator to the operand
     * @throws std::runtime_error if unknown operator
     */
    int64_t Evaluate(const SymbolTable& symbols) const override {
        int64_t val = expr->Evaluate(symbols);

        switch (operation) {
            case UnaryOp::Negate:
                return -val;
            case UnaryOp::BitwiseNot:
                return ~val;
            default:
                throw std::runtime_error("Unknown unary operator");
        }
    }

    /**
     * @brief Check if constant
     * @return Same as operand's IsConstant()
     */
    bool IsConstant() const override {
        return expr->IsConstant();
    }

    /**
     * @brief Check if relocatable
     * @return Same as operand's IsRelocatable()
     */
    bool IsRelocatable() const override {
        return expr->IsRelocatable();
    }

    /**
     * @brief Get the operator
     * @return The unary operator
     */
    UnaryOp GetOperation() const { return operation; }
    
    /**
     * @brief Get the operand
     * @return Shared pointer to operand expression
     */
    const std::shared_ptr<Expression>& GetOperand() const { return expr; }

private:
    UnaryOp operation;              ///< The unary operator
    std::shared_ptr<Expression> expr; ///< The operand
};

} // namespace xasm
