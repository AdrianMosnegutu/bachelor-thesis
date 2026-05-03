#include "dsl/lowerer/detail/ast_lowerer.hpp"
#include "dsl/lowerer/detail/expression_evaluator.hpp"

namespace dsl::lowerer::detail {

void lower_assign_statement(const ast::AssignStatement& stmt, const source::Location& loc, LowererContext& ctx) {
    ctx.assign(ctx.analysis().get_assign_target(stmt), evaluate_expression(*stmt.value, ctx), loc);
}

void lower_let_statement(const ast::LetStatement& stmt, LowererContext& ctx) {
    const auto* symbol = ctx.analysis().get_symbol_by_declaration(&stmt);
    if (!symbol) {
        throw LoweringFailure(stmt.value->location, "lowering reached let statement with no symbol annotation");
    }

    ctx.bind(symbol->id, evaluate_expression(*stmt.value, ctx));
}

}  // namespace dsl::lowerer::detail
