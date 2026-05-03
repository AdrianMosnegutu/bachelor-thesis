#include "dsl/common/ast/statements.hpp"
#include "dsl/common/utils/overloaded.hpp"
#include "dsl/lowerer/detail/ast_lowerer.hpp"
#include "dsl/lowerer/detail/lowerer_context.hpp"

namespace dsl::lowerer::detail {

ir::NoteEvents lower_statement(const ast::Statement& stmt, LowererContext& ctx, double& cursor) {
    const source::Location& loc = stmt.location;

    return std::visit(utils::overloaded{
                          [&](const ast::PlayStatement& s) { return lower_play_statement(s, ctx, cursor); },
                          [&](const ast::ForStatement& s) { return lower_for_statement(s, loc, ctx, cursor); },
                          [&](const ast::LoopStatement& s) { return lower_loop_statement(s, loc, ctx, cursor); },
                          [&](const ast::IfStatement& s) { return lower_if_statement(s, ctx, cursor); },
                          [&](const ast::LetStatement& s) {
                              lower_let_statement(s, ctx);
                              return ir::NoteEvents{};
                          },
                          [&](const ast::AssignStatement& s) {
                              lower_assign_statement(s, loc, ctx);
                              return ir::NoteEvents{};
                          },
                      },
                      stmt.kind);
}

ir::NoteEvents lower_block(const ast::Block& block, LowererContext& ctx, double& cursor) {
    ir::NoteEvents events;
    LowererScopeGuard scope(ctx);

    for (const auto& stmt_ptr : block) {
        try {
            auto inner_events = lower_statement(*stmt_ptr, ctx, cursor);
            events.insert(events.end(), inner_events.begin(), inner_events.end());
        } catch (const LoweringFailure& error) {
            ctx.report_lowering_error(error.what());
        }
    }

    return events;
}

}  // namespace dsl::lowerer::detail
