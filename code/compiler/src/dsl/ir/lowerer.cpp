#include "dsl/ir/lowerer.hpp"

#include "dsl/ast/statement.hpp"
#include "dsl/ir/ast_lowerer.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::ir {

Program lower(const semantic::AnalysisResult& analysis) {
    const ast::Program& program = analysis.program();

    Program out;
    lower_header(program.header, out);

    LowererContext ctx;
    ctx.collect_patterns(program.globals);
    ctx.execute_block = [&ctx](const ast::Block& b, double& cur) { return lower_block(b, ctx, cur); };

    ctx.push_scope();
    for (const auto& item : program.globals) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            if (const auto* let = std::get_if<ast::LetStatement>(&(*stmt_ptr)->kind)) {
                lower_let_statement(*let, ctx);
            }
        }
    }

    for (const auto& track : program.tracks) {
        out.tracks.push_back(lower_track_definition(track, ctx));
    }

    ctx.pop_scope();
    return out;
}

}  // namespace dsl::ir
