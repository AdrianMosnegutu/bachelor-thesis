#include "dsl/ir/ast_lowerer.hpp"
#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir {

void lower_assign_statement(const ast::AssignStatement& stmt, const Location& loc, LowererContext& ctx) {
    ctx.assign(stmt.name, evaluate_expression(*stmt.value, ctx), loc);
}

}  // namespace dsl::ir
