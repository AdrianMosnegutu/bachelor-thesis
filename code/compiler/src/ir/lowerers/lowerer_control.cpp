#include "dsl/ast/stmt.hpp"
#include "dsl/errors/semantic_error.hpp"
#include "dsl/ir/expression_evaluator.hpp"
#include "dsl/ir/lowerer.hpp"
#include "dsl/ir/lowerer_context.hpp"

namespace dsl::ir {

using errors::SemanticError;

std::vector<NoteEvent> Lowerer::lower_for(const ast::ForStatement& stmt, LowererContext& ctx, double& cursor) {
    ctx.push_scope();

    if (stmt.init) lower_stmt(*stmt.init, ctx, cursor);

    std::vector<NoteEvent> events;
    int iterations = 0;

    auto eval_cond = [&]() -> bool {
        if (!stmt.cond) return true;
        auto [kind] = evaluate_expression(*stmt.cond, ctx);
        if (const auto* b = std::get_if<bool>(&kind)) return *b;
        throw SemanticError(stmt.cond->loc, "for condition must be a boolean");
    };

    while (eval_cond()) {
        if (++iterations > LowererContext::MAX_ITERATIONS) {
            throw SemanticError(stmt.loc,
                                "for loop exceeded " + std::to_string(LowererContext::MAX_ITERATIONS) + " iterations");
        }
        auto evs = lower_block(stmt.body, ctx, cursor);
        events.insert(events.end(), evs.begin(), evs.end());
        if (stmt.step) lower_stmt(*stmt.step, ctx, cursor);
    }

    ctx.pop_scope();
    return events;
}

std::vector<NoteEvent> Lowerer::lower_loop(const ast::LoopStatement& stmt, LowererContext& ctx, double& cursor) {
    auto [kind] = evaluate_expression(*stmt.count, ctx);
    int count = 0;
    if (const auto* i = std::get_if<int>(&kind)) {
        count = *i;
    } else {
        throw SemanticError(stmt.count->loc, "loop count must be an integer");
    }
    if (count < 0) throw SemanticError(stmt.loc, "loop count must be non-negative");
    if (count > LowererContext::MAX_ITERATIONS) {
        throw SemanticError(stmt.loc,
                            "loop count " + std::to_string(count) + " exceeds limit of " +
                                std::to_string(LowererContext::MAX_ITERATIONS));
    }

    std::vector<NoteEvent> events;
    for (int i = 0; i < count; ++i) {
        auto evs = lower_block(stmt.body, ctx, cursor);
        events.insert(events.end(), evs.begin(), evs.end());
    }
    return events;
}

std::vector<NoteEvent> Lowerer::lower_if(const ast::IfStatement& stmt, LowererContext& ctx, double& cursor) {
    auto [kind] = evaluate_expression(*stmt.cond, ctx);

    if (!std::holds_alternative<bool>(kind)) {
        throw SemanticError(stmt.cond->loc, "if condition must be a boolean");
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
