#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "dsl/ast/ops.hpp"
#include "dsl/location.hpp"
#include "dsl/music/note.hpp"

namespace dsl::ast {

struct Expression;
using ExpressionPtr = std::unique_ptr<Expression>;

struct IntLiteral {
    int value;
};
struct FloatLiteral {
    double value;
};
struct BoolLiteral {
    bool value;
};
struct NoteLiteral {
    music::Note value;
};
struct RestLiteral {};
struct Identifier {
    std::string name;
};

struct UnaryExpression {
    UnaryOperator op;
    ExpressionPtr operand;
};
struct BinaryExpression {
    BinaryOperator op;
    ExpressionPtr lhs;
    ExpressionPtr rhs;
};
struct TernaryExpression {
    ExpressionPtr cond;
    ExpressionPtr then_expr;
    ExpressionPtr else_expr;
};
struct ParenthesisedExpression {
    ExpressionPtr inner;
};

// `duration` is null when no `:dur` suffix was written.
struct SequenceItem {
    ExpressionPtr value;
    ExpressionPtr duration;
};

struct Sequence {
    std::vector<SequenceItem> items;
};

// Grammar guarantees ≥2 elements. Items may carry their own `:duration`.
struct Chord {
    std::vector<SequenceItem> notes;
};

struct Call {
    std::string callee;
    std::vector<ExpressionPtr> args;
};

using ExpressionKind = std::variant<IntLiteral,
                                    FloatLiteral,
                                    BoolLiteral,
                                    NoteLiteral,
                                    RestLiteral,
                                    Identifier,
                                    UnaryExpression,
                                    BinaryExpression,
                                    TernaryExpression,
                                    ParenthesisedExpression,
                                    Sequence,
                                    Chord,
                                    Call>;

struct Expression {
    ExpressionKind kind;
    Location loc;
};

}  // namespace dsl::ast
