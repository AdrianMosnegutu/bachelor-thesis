#include "dsl/ast/statements.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/lowerer/detail/ast_lowerer.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::lowerer {

ir::Program lower(const semantic::AnalysisResult& analysis) {
    const auto& [header, globals, tracks] = analysis.program();

    ir::Program out;
    detail::lower_header(header, out);

    detail::LowererContext ctx;
    ctx.collect_patterns(globals);
    ctx.execute_block = [&ctx](const ast::Block& b, double& cur) { return detail::lower_block(b, ctx, cur); };

    ctx.push_scope();
    for (const auto& item : globals) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            if (const auto* let = std::get_if<ast::LetStatement>(&(*stmt_ptr)->kind)) {
                detail::lower_let_statement(*let, ctx);
            }
        }
    }

    for (const auto& track : tracks) {
        out.tracks.push_back(detail::lower_track_definition(track, ctx));
    }

    ctx.pop_scope();
    return out;
}

}  // namespace dsl::lowerer
