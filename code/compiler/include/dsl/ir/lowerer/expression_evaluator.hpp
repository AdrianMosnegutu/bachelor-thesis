#pragma once

#include "dsl/core/ast/expression.hpp"
#include "dsl/core/ast/expressions/literal_expressions.h"
#include "dsl/ir/lowerer/lowerer_context.hpp"
#include "dsl/ir/value.hpp"

namespace dsl::ir {

namespace detail {

Value evaluate_literal_expression(const ast::IntLiteralExpression& literal);
Value evaluate_literal_expression(const ast::FloatLiteralExpression& literal);
Value evaluate_literal_expression(const ast::BoolLiteralExpression& literal);
Value evaluate_literal_expression(const ast::RestLiteralExpression& literal);
Value evaluate_literal_expression(const ast::NoteLiteralExpression& literal);

Value evaluate_unary_expression(const ast::UnaryExpression& unary, const Location& loc, LowererContext& context);
Value evaluate_binary_expression(const ast::BinaryExpression& binary, const Location& loc, LowererContext& context);
Value evaluate_ternary_expression(const ast::TernaryExpression& ternary, const Location& loc, LowererContext& context);

Value evaluate_identifier_expression(const ast::IdentifierExpression& identifier,
                                     const Location& loc,
                                     const LowererContext& context);

Value evaluate_chord_expression(const ast::ChordExpression& chord, const Location& loc, LowererContext& context);
Value evaluate_sequence_expression(const ast::SequenceExpression& sequence, LowererContext& context);
Value evaluate_call_expression(const ast::CallExpression& call, const Location& loc, LowererContext& context);

}  // namespace detail

[[nodiscard]] Value evaluate_expression(const ast::Expression& expression, LowererContext& context);

}  // namespace dsl::ir
