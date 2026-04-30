#include "dsl/ir/ast_lowerer.hpp"

namespace dsl::ir {

NoteEvents lower_block(const ast::Block& block, LowererContext& ctx, double& cursor) {
    NoteEvents events;
    ctx.push_scope();

    for (const auto& stmt_ptr : block) {
        auto inner_events = lower_statement(*stmt_ptr, ctx, cursor);
        events.insert(events.end(), inner_events.begin(), inner_events.end());
    }

    ctx.pop_scope();
    return events;
}

}  // namespace dsl::ir
