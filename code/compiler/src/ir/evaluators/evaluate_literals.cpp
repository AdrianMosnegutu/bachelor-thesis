#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir::detail {

Value evaluate_literal(const ast::IntLiteral& literal) { return Value(literal.value); }

Value evaluate_literal(const ast::FloatLiteral& literal) { return Value(literal.value); }

Value evaluate_literal(const ast::BoolLiteral& literal) { return Value(literal.value); }

Value evaluate_literal(const ast::RestLiteral&) { return Value(RestVal(1.0)); }

Value evaluate_literal(const ast::NoteLiteral& literal) {
    return Value(NoteVal(literal.value.midi_number(), 1.0, 100));
}

}  // namespace dsl::ir::detail
