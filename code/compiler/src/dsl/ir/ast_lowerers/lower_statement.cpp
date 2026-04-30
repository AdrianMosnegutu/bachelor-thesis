#include "dsl/ir/ast_lowerer.hpp"
#include "dsl/utils/overloaded.hpp"

namespace dsl::ir {

NoteEvents lower_statement(const ast::Statement& stmt, LowererContext& ctx, double& cursor) {
    const Location& loc = stmt.location;

    return std::visit(utils::overloaded{
                          [&](const ast::PlayStatement& s) { return lower_play_statement(s, ctx, cursor); },
                          [&](const ast::ForStatement& s) { return lower_for_statement(s, loc, ctx, cursor); },
                          [&](const ast::LoopStatement& s) { return lower_loop_statement(s, loc, ctx, cursor); },
                          [&](const ast::IfStatement& s) { return lower_if_statement(s, loc, ctx, cursor); },
                          [&](const ast::LetStatement& s) {
                              lower_let_statement(s, ctx);
                              return NoteEvents{};
                          },
                          [&](const ast::AssignStatement& s) {
                              lower_assign_statement(s, loc, ctx);
                              return NoteEvents{};
                          },
                      },
                      stmt.kind);
}

}  // namespace dsl::ir
