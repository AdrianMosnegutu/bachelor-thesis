#include "dsl/lowerer/detail/ast_lowerer.hpp"
#include "dsl/lowerer/detail/expression_evaluator.hpp"

namespace dsl::lowerer::detail {

void lower_assign_statement(const ast::AssignStatement& stmt, const Location& loc, LowererContext& ctx) {
    ctx.assign(stmt.name, evaluate_expression(*stmt.value, ctx), loc);
}

void lower_let_statement(const ast::LetStatement& stmt, LowererContext& ctx) {
    ctx.bind(stmt.name, evaluate_expression(*stmt.value, ctx));
}

}  // namespace dsl::lowerer::detail