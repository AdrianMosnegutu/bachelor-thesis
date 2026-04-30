#include "dsl/errors/lowerer_error.hpp"
#include "dsl/ir/ast_lowerer.hpp"
#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir {

NoteEvents lower_if_statement(const ast::IfStatement& stmt, const Location& loc, LowererContext& ctx, double& cursor) {
    auto [kind] = evaluate_expression(*stmt.condition, ctx);

    if (!std::holds_alternative<bool>(kind)) {
        throw errors::LowererError(loc, "lowering reached if statement with a non-boolean condition");
    }

    if (std::get<bool>(kind)) {
        return lower_block(stmt.then_branch, ctx, cursor);
    }

    if (stmt.else_branch) {
        return lower_block(*stmt.else_branch, ctx, cursor);
    }

    return {};
}

}  // namespace dsl::ir
