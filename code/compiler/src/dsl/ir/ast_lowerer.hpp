#pragma once

#include "dsl/ir/lowerer_context.hpp"
#include "dsl/ir/program.hpp"

namespace dsl::ir {

void lower_header(const ast::Header& header, Program& out);

NoteEvents lower_block(const ast::Block& block, LowererContext& ctx, double& cursor);

Track lower_track_definition(const ast::TrackDefinition& track, LowererContext& ctx);

NoteEvents lower_voice_definition(const ast::VoiceDefinition& voice, LowererContext& ctx, double outer_cursor);

void lower_let_statement(const ast::LetStatement& stmt, LowererContext& ctx);

void lower_assign_statement(const ast::AssignStatement& stmt, const Location& loc, LowererContext& ctx);

NoteEvents lower_for_statement(const ast::ForStatement& stmt, const Location& loc, LowererContext& ctx, double& cursor);

NoteEvents lower_loop_statement(const ast::LoopStatement& stmt,
                                const Location& loc,
                                LowererContext& ctx,
                                double& cursor);

NoteEvents lower_if_statement(const ast::IfStatement& stmt, const Location& loc, LowererContext& ctx, double& cursor);

NoteEvents lower_play_statement(const ast::PlayStatement& stmt, LowererContext& ctx, double& cursor);

NoteEvents lower_statement(const ast::Statement& stmt, LowererContext& ctx, double& cursor);

}  // namespace dsl::ir
