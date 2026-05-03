#include "dsl/lowering/detail/ast_lowerer.hpp"
#include "dsl/lowering/detail/expression_evaluator.hpp"

namespace dsl::lowering::detail {

ir::NoteEvents lower_loop_statement(const ast::LoopStatement& stmt,
                                    const source::Location& loc,
                                    LowererContext& ctx,
                                    double& cursor) {
    const int count = std::get<int>(evaluate_expression(*stmt.count, ctx).kind);

    if (count < 0) {
        throw LoweringFailure(loc, "loop count must be non-negative");
    }

    if (count > LowererContext::MAX_ITERATIONS) {
        throw LoweringFailure(loc,
                              "loop count " + std::to_string(count) + " exceeds limit of " +
                                  std::to_string(LowererContext::MAX_ITERATIONS));
    }

    ir::NoteEvents events;
    for (int i = 0; i < count; ++i) {
        auto inner_events = lower_block(stmt.body, ctx, cursor);
        events.insert(events.end(), inner_events.begin(), inner_events.end());
    }

    return events;
}

ir::NoteEvents lower_for_statement(const ast::ForStatement& stmt,
                                   const source::Location& loc,
                                   LowererContext& ctx,
                                   double& cursor) {
    LowererScopeGuard scope(ctx);

    if (stmt.init) {
        lower_statement(*stmt.init, ctx, cursor);
    }

    ir::NoteEvents events;
    int iterations = 0;

    auto evaluate_condition = [&]() -> bool {
        if (!stmt.condition) {
            return true;
        }

        return std::get<bool>(evaluate_expression(*stmt.condition, ctx).kind);
    };

    while (evaluate_condition()) {
        if (++iterations > LowererContext::MAX_ITERATIONS) {
            throw LoweringFailure(
                loc,
                "for loop exceeded " + std::to_string(LowererContext::MAX_ITERATIONS) + " iterations");
        }

        auto inner_events = lower_block(stmt.body, ctx, cursor);
        events.insert(events.end(), inner_events.begin(), inner_events.end());

        if (stmt.step) {
            lower_statement(*stmt.step, ctx, cursor);
        }
    }

    return events;
}

ir::NoteEvents lower_if_statement(const ast::IfStatement& stmt, LowererContext& ctx, double& cursor) {
    if (std::get<bool>(evaluate_expression(*stmt.condition, ctx).kind)) {
        return lower_block(stmt.then_branch, ctx, cursor);
    }

    if (stmt.else_branch) {
        return lower_block(*stmt.else_branch, ctx, cursor);
    }

    return {};
}

}  // namespace dsl::lowering::detail
