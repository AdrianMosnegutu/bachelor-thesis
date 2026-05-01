#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "dsl/ast/operators.hpp"
#include "dsl/music/note.hpp"
#include "dsl/source/location.hpp"

namespace dsl::ast {

struct Expression;
using ExpressionPtr = std::unique_ptr<Expression>;

// -- Composable expressions -------------------------------------------------------------------------------------------

struct UnaryExpression {
    UnaryOperator operation;
    ExpressionPtr operand;
};

struct BinaryExpression {
    BinaryOperator operation;
    ExpressionPtr left;
    ExpressionPtr right;
};

struct TernaryExpression {
    ExpressionPtr condition;
    ExpressionPtr then_expression;
    ExpressionPtr else_expression;
};

// -- Literals ---------------------------------------------------------------------------------------------------------

struct IntLiteralExpression {
    int value;
};

struct FloatLiteralExpression {
    double value;
};

struct BoolLiteralExpression {
    bool value;
};

struct NoteLiteralExpression {
    music::Note value;
};

struct RestLiteralExpression {};

// -- Musical structures -----------------------------------------------------------------------------------------------

struct DurationalTarget {
    ExpressionPtr value;
    ExpressionPtr duration;
};

struct ChordExpression {
    std::vector<DurationalTarget> notes;
};

struct SequenceExpression {
    std::vector<DurationalTarget> items;
};

// -- Others -----------------------------------------------------------------------------------------------------------

struct ParenthesisedExpression {
    ExpressionPtr inner;
};

struct IdentifierExpression {
    std::string name;
};

struct PatternCallExpression {
    std::string callee;
    std::vector<ExpressionPtr> arguments;
};

// -- Base expression --------------------------------------------------------------------------------------------------

using ExpressionKind = std::variant<IntLiteralExpression,
                                    FloatLiteralExpression,
                                    BoolLiteralExpression,
                                    NoteLiteralExpression,
                                    RestLiteralExpression,
                                    IdentifierExpression,
                                    UnaryExpression,
                                    BinaryExpression,
                                    TernaryExpression,
                                    ParenthesisedExpression,
                                    SequenceExpression,
                                    ChordExpression,
                                    PatternCallExpression>;

struct Expression {
    ExpressionKind kind;
    source::Location location;
};

}  // namespace dsl::ast
