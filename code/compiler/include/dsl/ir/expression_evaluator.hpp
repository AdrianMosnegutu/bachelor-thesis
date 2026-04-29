#pragma once

#include "dsl/core/ast/expression.hpp"
#include "dsl/core/ast/expressions/literal_expressions.h"
#include "dsl/ir/lowerer_context.hpp"
#include "dsl/ir/value.hpp"

namespace dsl::ir {

class LowererContext;

namespace detail {

Value evaluate_literal(const ast::IntLiteralExpression& literal);
Value evaluate_literal(const ast::FloatLiteralExpression& literal);
Value evaluate_literal(const ast::BoolLiteralExpression& literal);
Value evaluate_literal(const ast::RestLiteralExpression& literal);
Value evaluate_literal(const ast::NoteLiteralExpression& literal);

Value evaluate_unary(const ast::UnaryExpression& unary, const Location& loc, LowererContext& context);
Value evaluate_binary(const ast::BinaryExpression& binary, const Location& loc, LowererContext& context);
Value evaluate_ternary(const ast::TernaryExpression& ternary, const Location& loc, LowererContext& context);

Value evaluate_identifier(const ast::IdentifierExpression& identifier,
                          const Location& loc,
                          const LowererContext& context);

Value evaluate_chord(const ast::ChordExpression& chord, const Location& loc, LowererContext& context);
Value evaluate_sequence(const ast::SequenceExpression& sequence, LowererContext& context);
Value evaluate_call(const ast::CallExpression& call, const Location& loc, LowererContext& context);

}  // namespace detail

[[nodiscard]] Value evaluate_expression(const ast::Expression& expression, LowererContext& context);

}  // namespace dsl::ir
