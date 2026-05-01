#include "dsl/ast/expressions.hpp"
#include "dsl/errors/lowerer_error.hpp"
#include "dsl/ir/values.hpp"
#include "dsl/lowerer/detail/expression_evaluator.hpp"
#include "dsl/lowerer/detail/lowerer_context.hpp"

namespace dsl::lowerer::detail {

ir::Value evaluate_ternary_expression(const ast::TernaryExpression& ternary,
                                      const source::Location& loc,
                                      LowererContext& context) {
    const auto [kind] = evaluate_expression(*ternary.condition, context);
    const auto* cond_bool = std::get_if<bool>(&kind);
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

}  // namespace dsl::lowerer::detail
