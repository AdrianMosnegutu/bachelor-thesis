#pragma once

#include "dsl/ast/expr.hpp"
#include "dsl/ir/lowerer_context.hpp"
#include "dsl/ir/value.hpp"
#include "dsl/location.hpp"

namespace dsl::ir {

class LowererContext;

namespace detail {

Value evaluate_literal(const ast::IntLiteral& literal);
Value evaluate_literal(const ast::FloatLiteral& literal);
Value evaluate_literal(const ast::BoolLiteral& literal);
Value evaluate_literal(const ast::RestLiteral& literal);
Value evaluate_literal(const ast::NoteLiteral& literal);

Value evaluate_unary(const ast::UnaryExpression& unary, const Location& loc, LowererContext& context);
Value evaluate_binary(const ast::BinaryExpression& binary, const Location& loc, LowererContext& context);
Value evaluate_ternary(const ast::TernaryExpression& ternary, const Location& loc, LowererContext& context);

Value evaluate_identifier(const ast::Identifier& identifier, const Location& loc, const LowererContext& context);

Value evaluate_chord(const ast::Chord& chord, const Location& loc, LowererContext& context);
Value evaluate_sequence(const ast::Sequence& sequence, LowererContext& context);
Value evaluate_call(const ast::Call& call, const Location& loc, LowererContext& context);

}  // namespace detail

[[nodiscard]] Value evaluate_expression(const ast::Expression& expression, LowererContext& context);

}  // namespace dsl::ir
