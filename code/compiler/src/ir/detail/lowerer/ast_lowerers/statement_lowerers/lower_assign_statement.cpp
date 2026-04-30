#include "dsl/ir/detail/expression_evaluator.hpp"
#include "dsl/ir/detail/lowerer/ast_lowerers.h"

namespace dsl::ir::detail {

void lower_assign_statement(const ast::AssignStatement& stmt, const Location& loc, LowererContext& ctx) {
    ctx.assign(stmt.name, evaluate_expression(*stmt.value, ctx), loc);
}

}  // namespace dsl::ir::detail
