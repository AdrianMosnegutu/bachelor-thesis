#include "dsl/ir/lowerer/expression_evaluator.hpp"
#include "dsl/ir/lowerer/lowerer.hpp"

namespace dsl::ir {

void Lowerer::lower_let(const ast::LetStatement& stmt, LowererContext& ctx) {
    ctx.bind(stmt.name, evaluate_expression(*stmt.value, ctx));
}

void Lowerer::lower_assign(const ast::AssignStatement& stmt, const Location& loc, LowererContext& ctx) {
    ctx.assign(stmt.name, evaluate_expression(*stmt.value, ctx), loc);
}

}  // namespace dsl::ir
