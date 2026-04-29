#include "dsl/errors/semantic_error.hpp"
#include "dsl/ir/expression_evaluator.hpp"
#include "dsl/ir/value.hpp"
#include "dsl/location.hpp"

namespace dsl::ir::detail {

using errors::SemanticError;

namespace {

Value evaluate_negative(const ValueKind& operand, const Location& loc) {
    if (auto* integer = std::get_if<int>(&operand)) {
        return Value{-*integer};
    }

    if (auto* floating_point = std::get_if<double>(&operand)) {
        return Value{-*floating_point};
    }

    throw SemanticError(loc, "unary '-' requires a numeric operand");
}

Value evaluate_not(const ValueKind& operand, const Location& loc) {
    if (auto* boolean = std::get_if<bool>(&operand)) {
        return Value{!*boolean};
    }

    throw SemanticError(loc, "unary '!' requires a boolean operand");
}

}  // namespace

Value evaluate_unary(const ast::UnaryExpression& unary, const Location& loc, LowererContext& context) {
    const ValueKind operand = evaluate_expression(*unary.operand, context).kind;

    switch (unary.op) {
        case ast::UnaryOperator::Negative:
            return evaluate_negative(operand, loc);
        case ast::UnaryOperator::Not:
            return evaluate_not(operand, loc);
    }

    throw SemanticError(loc, "invalid unary operator");
}

}  // namespace dsl::ir::detail
