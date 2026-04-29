#pragma once

#include "dsl/core/ast/ast.h"
#include "dsl/core/ast/program.hpp"
#include "dsl/core/ast/statement.hpp"
#include "dsl/core/location.hpp"
#include "dsl/ir/lowerer_context.hpp"
#include "dsl/ir/program.hpp"

namespace dsl::ir {

class Lowerer {
   public:
    ProgramIR lower(const ast::Program& program);

   private:
    static void lower_header(const ast::Header& header, ProgramIR& out);

    TrackIR lower_track(const ast::TrackDefinition& track, LowererContext& ctx);
    std::vector<NoteEvent> lower_voice(const ast::VoiceDefinition& voice, LowererContext& ctx, double outer_cursor);
    std::vector<NoteEvent> lower_block(const ast::Block& block, LowererContext& ctx, double& cursor);
    std::vector<NoteEvent> lower_stmt(const ast::Statement& stmt, LowererContext& ctx, double& cursor);
    static std::vector<NoteEvent> lower_play(const ast::PlayStatement& stmt, LowererContext& ctx, double& cursor);
    std::vector<NoteEvent> lower_for(const ast::ForStatement& stmt,
                                     const Location& loc,
                                     LowererContext& ctx,
                                     double& cursor);
    std::vector<NoteEvent> lower_loop(const ast::LoopStatement& stmt,
                                      const Location& loc,
                                      LowererContext& ctx,
                                      double& cursor);
    std::vector<NoteEvent> lower_if(const ast::IfStatement& stmt,
                                    const Location& loc,
                                    LowererContext& ctx,
                                    double& cursor);
    static void lower_let(const ast::LetStatement& stmt, LowererContext& ctx);
    static void lower_assign(const ast::AssignStatement& stmt, const Location& loc, LowererContext& ctx);
};

}  // namespace dsl::ir
