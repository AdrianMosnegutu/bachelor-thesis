#pragma once

#include "dsl/ast/decl.hpp"
#include "dsl/ast/program.hpp"
#include "dsl/ast/stmt.hpp"
#include "dsl/ir/lowerer_context.hpp"
#include "dsl/ir/program.hpp"

namespace dsl::ir {

class Lowerer {
   public:
    ProgramIR lower(const ast::Program& program);

   private:
    static void lower_header(const ast::Header& header, ProgramIR& out);

    TrackIR lower_track(const ast::TrackDeclaration& track, LowererContext& ctx);
    std::vector<NoteEvent> lower_voice(const ast::VoiceDeclaration& voice, LowererContext& ctx, double outer_cursor);
    std::vector<NoteEvent> lower_block(const ast::Block& block, LowererContext& ctx, double& cursor);
    std::vector<NoteEvent> lower_stmt(const ast::Statement& stmt, LowererContext& ctx, double& cursor);
    static std::vector<NoteEvent> lower_play(const ast::PlayStatement& stmt, LowererContext& ctx, double& cursor);
    std::vector<NoteEvent> lower_for(const ast::ForStatement& stmt, LowererContext& ctx, double& cursor);
    std::vector<NoteEvent> lower_loop(const ast::LoopStatement& stmt, LowererContext& ctx, double& cursor);
    std::vector<NoteEvent> lower_if(const ast::IfStatement& stmt, LowererContext& ctx, double& cursor);
    static void lower_let(const ast::LetStatement& stmt, LowererContext& ctx);
    static void lower_assign(const ast::AssignStatement& stmt, LowererContext& ctx);
};

}  // namespace dsl::ir
