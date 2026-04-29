#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir::detail {

Value evaluate_literal(const ast::IntLiteralExpression& literal) { return Value(literal.value); }

Value evaluate_literal(const ast::FloatLiteralExpression& literal) { return Value(literal.value); }

Value evaluate_literal(const ast::BoolLiteralExpression& literal) { return Value(literal.value); }

Value evaluate_literal(const ast::RestLiteralExpression&) { return Value(RestVal(1.0)); }

Value evaluate_literal(const ast::NoteLiteralExpression& literal) {
    return Value(NoteVal(literal.value.midi_number(), 1.0, 100));
}

}  // namespace dsl::ir::detail
