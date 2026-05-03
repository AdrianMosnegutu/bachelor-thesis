#include "dsl/common/ast/expressions.hpp"
#include "dsl/common/ir/values.hpp"
#include "dsl/lowering/detail/expression_evaluator.hpp"
#include "dsl/lowering/detail/lowerer_context.hpp"

namespace dsl::lowering::detail {

ir::Value evaluate_ternary_expression(const ast::TernaryExpression& ternary, LowererContext& context) {
    const bool cond = std::get<bool>(evaluate_expression(*ternary.condition, context).kind);
    return cond ? evaluate_expression(*ternary.then_expression, context)
                : evaluate_expression(*ternary.else_expression, context);
}

}  // namespace dsl::lowering::detail
