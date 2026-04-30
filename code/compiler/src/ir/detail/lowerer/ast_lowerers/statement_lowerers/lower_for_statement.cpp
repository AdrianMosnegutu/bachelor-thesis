#include "dsl/core/errors/lowerer_error.hpp"
#include "dsl/ir/detail/expression_evaluator.hpp"
#include "dsl/ir/detail/lowerer/ast_lowerers.h"

namespace dsl::ir::detail {

using errors::LowererError;

NoteEvents lower_for_statement(const ast::ForStatement& for_stmt,
                               const Location& loc,
                               LowererContext& ctx,
                               double& cursor) {
    ctx.push_scope();

    if (for_stmt.init) {
        lower_statement(*for_stmt.init, ctx, cursor);
    }

    NoteEvents events;
    int iterations = 0;

    auto evaluate_condition = [&]() -> bool {
        if (!for_stmt.condition) {
            return true;
        }

        auto [kind] = evaluate_expression(*for_stmt.condition, ctx);
        if (const auto* boolean = std::get_if<bool>(&kind)) {
            return *boolean;
        }

        throw LowererError(loc, "lowering reached for statement with a non-boolean condition");
    };

    while (evaluate_condition()) {
        if (++iterations > LowererContext::MAX_ITERATIONS) {
            throw LowererError(loc,
                               "for loop exceeded " + std::to_string(LowererContext::MAX_ITERATIONS) + " iterations");
        }

        auto inner_events = lower_block(for_stmt.body, ctx, cursor);
        events.insert(events.end(), inner_events.begin(), inner_events.end());

        if (for_stmt.step) {
            lower_statement(*for_stmt.step, ctx, cursor);
        }
    }

    ctx.pop_scope();
    return events;
}

}  // namespace dsl::ir::detail
