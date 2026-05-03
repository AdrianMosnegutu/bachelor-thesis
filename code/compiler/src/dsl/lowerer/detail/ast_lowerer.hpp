#pragma once

#include "dsl/common/ast/declarations.hpp"
#include "dsl/common/ast/statements.hpp"
#include "dsl/common/ir/program.hpp"
#include "lowerer_context.hpp"

namespace dsl::lowerer::detail {

void lower_let_statement(const ast::LetStatement& stmt, LowererContext& ctx);

void lower_assign_statement(const ast::AssignStatement& stmt, const source::Location& loc, LowererContext& ctx);

ir::NoteEvents lower_for_statement(const ast::ForStatement& stmt,
                                   const source::Location& loc,
                                   LowererContext& ctx,
                                   double& cursor);

ir::NoteEvents lower_loop_statement(const ast::LoopStatement& stmt,
                                    const source::Location& loc,
                                    LowererContext& ctx,
                                    double& cursor);

ir::NoteEvents lower_if_statement(const ast::IfStatement& stmt, LowererContext& ctx, double& cursor);

ir::NoteEvents lower_play_statement(const ast::PlayStatement& stmt, LowererContext& ctx, double& cursor);

void lower_header(const ast::Header& header, ir::Program& out);

ir::NoteEvents lower_block(const ast::Block& block, LowererContext& ctx, double& cursor);

ir::Track lower_track_definition(const ast::TrackDefinition& track, LowererContext& ctx);

ir::NoteEvents lower_voice_definition(const ast::VoiceDefinition& voice, LowererContext& ctx, double outer_cursor);

ir::NoteEvents lower_statement(const ast::Statement& stmt, LowererContext& ctx, double& cursor);

}  // namespace dsl::lowerer::detail
