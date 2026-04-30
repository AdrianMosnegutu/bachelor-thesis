#include "dsl/core/errors/lowerer_error.hpp"
#include "dsl/ir/detail/expression_evaluator.hpp"

namespace dsl::ir::detail {

Value evaluate_ternary_expression(const ast::TernaryExpression& ternary, const Location& loc, LowererContext& context) {
    const auto cond_val = evaluate_expression(*ternary.condition, context);
    const auto* cond_bool = std::get_if<bool>(&cond_val.kind);
    if (!cond_bool) {
        throw errors::LowererError(loc, "lowering reached ternary with a non-boolean condition");
    }

    const auto then_val = evaluate_expression(*ternary.then_expression, context);
    const auto else_val = evaluate_expression(*ternary.else_expression, context);

    if (then_val.kind.index() != else_val.kind.index()) {
        throw errors::LowererError(loc, "lowering reached ternary with mismatched branch types");
    }

    return *cond_bool ? then_val : else_val;
}

}  // namespace dsl::ir::detail
