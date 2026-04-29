#include "dsl/core/ast/statement.hpp"
#include "dsl/ir/expression_evaluator.hpp"
#include "dsl/ir/lowerer.hpp"
#include "dsl/ir/lowerer_context.hpp"

namespace dsl::ir {

void Lowerer::lower_let(const ast::LetStatement& stmt, LowererContext& ctx) {
    ctx.bind(stmt.name, evaluate_expression(*stmt.value, ctx));
}

void Lowerer::lower_assign(const ast::AssignStatement& stmt, const Location& loc, LowererContext& ctx) {
    ctx.assign(stmt.name, evaluate_expression(*stmt.value, ctx), loc);
}

}  // namespace dsl::ir
