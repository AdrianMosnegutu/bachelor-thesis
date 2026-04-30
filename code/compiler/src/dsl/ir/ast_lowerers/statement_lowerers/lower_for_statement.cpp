#include "dsl/errors/lowerer_error.hpp"
#include "dsl/ir/ast_lowerer.hpp"
#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir {

using errors::LowererError;

NoteEvents lower_for_statement(const ast::ForStatement& stmt,
                               const Location& loc,
                               LowererContext& ctx,
                               double& cursor) {
    ctx.push_scope();

    if (stmt.init) {
        lower_statement(*stmt.init, ctx, cursor);
    }

    NoteEvents events;
    int iterations = 0;

    auto evaluate_condition = [&]() -> bool {
        if (!stmt.condition) {
            return true;
        }

        auto [kind] = evaluate_expression(*stmt.condition, ctx);
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

        auto inner_events = lower_block(stmt.body, ctx, cursor);
        events.insert(events.end(), inner_events.begin(), inner_events.end());

        if (stmt.step) {
            lower_statement(*stmt.step, ctx, cursor);
        }
    }

    ctx.pop_scope();
    return events;
}

}  // namespace dsl::ir
