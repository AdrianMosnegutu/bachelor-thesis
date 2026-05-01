#include "dsl/lowerer/detail/ast_lowerer.hpp"
#include "dsl/lowerer/detail/expression_evaluator.hpp"

namespace dsl::lowerer::detail {

ir::NoteEvents lower_loop_statement(const ast::LoopStatement& stmt,
                                    const source::Location& loc,
                                    LowererContext& ctx,
                                    double& cursor) {
    auto [kind] = evaluate_expression(*stmt.count, ctx);
    int count = 0;

    if (const auto* integer = std::get_if<int>(&kind)) {
        count = *integer;
    } else {
        throw LoweringFailure(loc, "lowering reached loop statement with a non-integer count");
    }

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

        auto [kind] = evaluate_expression(*stmt.condition, ctx);
        if (const auto* boolean = std::get_if<bool>(&kind)) {
            return *boolean;
        }

        throw LoweringFailure(loc, "lowering reached for statement with a non-boolean condition");
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

ir::NoteEvents lower_if_statement(const ast::IfStatement& stmt,
                                  const source::Location& loc,
                                  LowererContext& ctx,
                                  double& cursor) {
    auto [kind] = evaluate_expression(*stmt.condition, ctx);

    if (!std::holds_alternative<bool>(kind)) {
        throw LoweringFailure(loc, "lowering reached if statement with a non-boolean condition");
    }

    if (std::get<bool>(kind)) {
        return lower_block(stmt.then_branch, ctx, cursor);
    }

    if (stmt.else_branch) {
        return lower_block(*stmt.else_branch, ctx, cursor);
    }

    return {};
}

}  // namespace dsl::lowerer::detail
