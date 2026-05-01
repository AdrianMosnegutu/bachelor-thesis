#include "dsl/ast/expressions.hpp"
#include "dsl/errors/lowerer_error.hpp"
#include "dsl/ir/values.hpp"
#include "dsl/lowerer/detail/expression_evaluator.hpp"
#include "dsl/lowerer/detail/lowerer_context.hpp"

namespace dsl::lowerer::detail {

namespace {

using ir::Value;
using ir::ValueKind;

Value evaluate_negative(const ValueKind& operand, const Location& loc) {
    if (auto* integer = std::get_if<int>(&operand)) {
        return Value{-*integer};
    }

    if (auto* floating_point = std::get_if<double>(&operand)) {
        return Value{-*floating_point};
    }

    throw errors::LowererError(loc, "lowering reached unary '-' with a non-numeric operand");
}

Value evaluate_not(const ValueKind& operand, const Location& loc) {
    if (auto* boolean = std::get_if<bool>(&operand)) {
        return Value{!*boolean};
    }

    throw errors::LowererError(loc, "lowering reached unary '!' with a non-boolean operand");
}

}  // namespace

Value evaluate_unary_expression(const ast::UnaryExpression& unary, const Location& loc, LowererContext& context) {
    const ValueKind operand = evaluate_expression(*unary.operand, context).kind;

    switch (unary.operation) {
        case ast::UnaryOperator::Negative:
            return evaluate_negative(operand, loc);
        case ast::UnaryOperator::Not:
            return evaluate_not(operand, loc);
    }

    throw errors::LowererError(loc, "lowering reached invalid unary operator");
}

}  // namespace dsl::lowerer::detail
