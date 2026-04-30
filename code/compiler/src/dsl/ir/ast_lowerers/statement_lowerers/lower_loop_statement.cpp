#include "dsl/errors/lowerer_error.hpp"
#include "dsl/ir/ast_lowerer.hpp"
#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir {

using errors::LowererError;

NoteEvents lower_loop_statement(const ast::LoopStatement& stmt,
                                const Location& loc,
                                LowererContext& ctx,
                                double& cursor) {
    auto [kind] = evaluate_expression(*stmt.count, ctx);
    int count = 0;

    if (const auto* integer = std::get_if<int>(&kind)) {
        count = *integer;
    } else {
        throw LowererError(loc, "lowering reached loop statement with a non-integer count");
    }

    if (count < 0) {
        throw LowererError(loc, "loop count must be non-negative");
    }

    if (count > LowererContext::MAX_ITERATIONS) {
        throw LowererError(loc,
                           "loop count " + std::to_string(count) + " exceeds limit of " +
                               std::to_string(LowererContext::MAX_ITERATIONS));
    }

    NoteEvents events;
    for (int i = 0; i < count; ++i) {
        auto inner_events = lower_block(stmt.body, ctx, cursor);
        events.insert(events.end(), inner_events.begin(), inner_events.end());
    }

    return events;
}

}  // namespace dsl::ir
