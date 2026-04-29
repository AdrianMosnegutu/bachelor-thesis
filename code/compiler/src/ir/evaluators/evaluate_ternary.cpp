#include "dsl/errors/semantic_error.hpp"
#include "dsl/ir/expression_evaluator.hpp"
#include "dsl/ir/value.hpp"
#include "dsl/location.hpp"

namespace dsl::ir::detail {

using errors::SemanticError;

Value evaluate_ternary(const ast::TernaryExpression& ternary, const Location& loc, LowererContext& context) {
    const auto cond_val = evaluate_expression(*ternary.cond, context);
    const auto* cond_bool = std::get_if<bool>(&cond_val.kind);
    if (!cond_bool) {
        throw SemanticError(loc, "ternary condition must be a boolean expression");
    }

    const auto then_val = evaluate_expression(*ternary.then_expr, context);
    const auto else_val = evaluate_expression(*ternary.else_expr, context);

    if (then_val.kind.index() != else_val.kind.index()) {
        throw SemanticError(loc, "ternary branches must evaluate to the same type");
    }

    return *cond_bool ? then_val : else_val;
}

}  // namespace dsl::ir::detail
