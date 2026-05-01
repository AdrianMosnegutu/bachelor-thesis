#include "dsl/common/ir/values.hpp"
#include "dsl/lowerer/detail/expression_evaluator.hpp"

namespace dsl::lowerer::detail {

namespace {

using ir::NoteValue;
using ir::RestValue;
using ir::Value;

}  // namespace

Value evaluate_literal_expression(const ast::IntLiteralExpression& literal) { return Value(literal.value); }

Value evaluate_literal_expression(const ast::FloatLiteralExpression& literal) { return Value(literal.value); }

Value evaluate_literal_expression(const ast::BoolLiteralExpression& literal) { return Value(literal.value); }

Value evaluate_literal_expression(const ast::RestLiteralExpression&) { return Value(RestValue(1.0)); }

Value evaluate_literal_expression(const ast::NoteLiteralExpression& literal) {
    return Value(NoteValue(literal.value.midi_number(), 1.0, 100));
}

}  // namespace dsl::lowerer::detail
