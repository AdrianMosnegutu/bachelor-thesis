#include "dsl/core/errors/semantic_error.hpp"
#include "dsl/ir/detail/expression_evaluator.hpp"
#include "dsl/ir/detail/lowerer/ast_lowerers.h"

namespace dsl::ir::detail {

using errors::SemanticError;

NoteEvents lower_if_statement(const ast::IfStatement& stmt, const Location&, LowererContext& ctx, double& cursor) {
    auto [kind] = evaluate_expression(*stmt.condition, ctx);

    if (!std::holds_alternative<bool>(kind)) {
        throw SemanticError(stmt.condition->location, "if condition must be a boolean");
    }

    if (std::get<bool>(kind)) {
        return lower_block(stmt.then_branch, ctx, cursor);
    }

    if (stmt.else_branch) {
        return lower_block(*stmt.else_branch, ctx, cursor);
    }

    return {};
}

}  // namespace dsl::ir::detail
