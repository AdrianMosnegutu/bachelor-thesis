#pragma once

#include <memory>
#include <variant>

#include "dsl/source/location.hpp"
#include "expressions/binary_expression.hpp"
#include "expressions/call_expression.hpp"
#include "expressions/chord_expression.hpp"
#include "expressions/identifier_expression.hpp"
#include "expressions/literal_expressions.hpp"
#include "expressions/parenthesised_expression.hpp"
#include "expressions/sequence_expression.hpp"
#include "expressions/ternary_expression.hpp"
#include "expressions/unary_expression.hpp"

namespace dsl::ast {

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
                                    CallExpression>;

struct Expression {
    ExpressionKind kind;
    Location location;
};

using ExpressionPtr = std::unique_ptr<Expression>;

}  // namespace dsl::ast
