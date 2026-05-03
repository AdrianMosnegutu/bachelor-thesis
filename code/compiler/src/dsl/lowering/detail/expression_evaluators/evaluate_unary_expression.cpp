#include "dsl/common/ast/expressions.hpp"
#include "dsl/common/ir/values.hpp"
#include "dsl/lowering/detail/expression_evaluator.hpp"
#include "dsl/lowering/detail/lowerer_context.hpp"

namespace dsl::lowering::detail {

namespace {

using ir::Value;
using ir::ValueKind;

}  // namespace

Value evaluate_unary_expression(const ast::UnaryExpression& unary,
                                const source::Location& loc,
                                LowererContext& context) {
    const ValueKind operand = evaluate_expression(*unary.operand, context).kind;

    switch (unary.operation) {
        case ast::UnaryOperator::Negative: {
            if (const auto* integer = std::get_if<int>(&operand)) {
                return Value{-*integer};
            }

            return Value{-std::get<double>(operand)};
        }
        case ast::UnaryOperator::Not: {
            return Value{!std::get<bool>(operand)};
        }
    }

    throw LoweringFailure(loc, "lowering reached invalid unary operator");
}

}  // namespace dsl::lowering::detail
