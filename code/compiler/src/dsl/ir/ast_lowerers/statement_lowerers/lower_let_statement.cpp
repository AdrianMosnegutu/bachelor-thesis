#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir {

void lower_let_statement(const ast::LetStatement& stmt, LowererContext& ctx) {
    ctx.bind(stmt.name, evaluate_expression(*stmt.value, ctx));
}

}  // namespace dsl::ir
