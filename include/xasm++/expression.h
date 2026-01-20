// Expression - Expression trees for complex expressions and forward references
// Phase 1: Minimal Viable Assembler - Core Abstractions

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <map>

namespace xasm {

// Forward declaration
class SymbolTable;

// Binary operators
enum class BinaryOp {
    Add,
    Subtract,
    Multiply,
    Divide,
    // More operators to be added in future phases:
    // Modulo, BitwiseAnd, BitwiseOr, BitwiseXor, ShiftLeft, ShiftRight, etc.
};

// Unary operators
enum class UnaryOp {
    Negate,
    BitwiseNot,
    // More operators to be added in future phases:
    // LogicalNot, etc.
};

// Base Expression class
class Expression {
public:
    virtual ~Expression() = default;

    // Evaluate the expression with the given symbol table
    virtual int64_t Evaluate(const SymbolTable& symbols) const = 0;

    // True if the expression is constant (no symbol references)
    virtual bool IsConstant() const = 0;

    // True if the expression contains relocatable symbols (addresses)
    virtual bool IsRelocatable() const = 0;
};

// Abstract SymbolTable interface
class SymbolTable {
public:
    virtual ~SymbolTable() = default;

    // Lookup a symbol by name, returns true if found
    virtual bool Lookup(const std::string& name, int64_t& value) const = 0;
};

// Literal expression - constant value
class LiteralExpr : public Expression {
public:
    explicit LiteralExpr(int64_t val) : value(val) {}

    int64_t Evaluate(const SymbolTable& /*symbols*/) const override {
        return value;
    }

    bool IsConstant() const override {
        return true;
    }

    bool IsRelocatable() const override {
        return false;
    }

    int64_t GetValue() const { return value; }

private:
    int64_t value;
};

// Symbol expression - symbol reference
class SymbolExpr : public Expression {
public:
    explicit SymbolExpr(const std::string& sym) : symbol(sym) {}

    int64_t Evaluate(const SymbolTable& symbols) const override {
        int64_t value;
        if (!symbols.Lookup(symbol, value)) {
            throw std::runtime_error("Undefined symbol: " + symbol);
        }
        return value;
    }

    bool IsConstant() const override {
        return false;  // Symbol reference is not constant
    }

    bool IsRelocatable() const override {
        return true;  // Symbol references are relocatable
    }

    const std::string& GetSymbol() const { return symbol; }

private:
    std::string symbol;
};

// Binary operation expression
class BinaryOpExpr : public Expression {
public:
    BinaryOpExpr(BinaryOp op, std::shared_ptr<Expression> l, std::shared_ptr<Expression> r)
        : operation(op), left(l), right(r) {}

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

    bool IsConstant() const override {
        return left->IsConstant() && right->IsConstant();
    }

    bool IsRelocatable() const override {
        return left->IsRelocatable() || right->IsRelocatable();
    }

    BinaryOp GetOperation() const { return operation; }
    const std::shared_ptr<Expression>& GetLeft() const { return left; }
    const std::shared_ptr<Expression>& GetRight() const { return right; }

private:
    BinaryOp operation;
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
};

// Unary operation expression
class UnaryOpExpr : public Expression {
public:
    UnaryOpExpr(UnaryOp op, std::shared_ptr<Expression> operand)
        : operation(op), expr(operand) {}

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

    bool IsConstant() const override {
        return expr->IsConstant();
    }

    bool IsRelocatable() const override {
        return expr->IsRelocatable();
    }

    UnaryOp GetOperation() const { return operation; }
    const std::shared_ptr<Expression>& GetOperand() const { return expr; }

private:
    UnaryOp operation;
    std::shared_ptr<Expression> expr;
};

} // namespace xasm
