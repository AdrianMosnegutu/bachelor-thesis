#pragma once

#include <memory>
#include <variant>

#include "dsl/core/location.hpp"
#include "expressions/binary_expression.h"
#include "expressions/call_expression.h"
#include "expressions/chord_expression.h"
#include "expressions/identifier_expression.h"
#include "expressions/literal_expressions.h"
#include "expressions/parenthesised_expression.h"
#include "expressions/sequence_expression.h"
#include "expressions/ternary_expression.h"
#include "expressions/unary_expression.h"

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
