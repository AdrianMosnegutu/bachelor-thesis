#include "dsl/lowerer/lowerer.hpp"

#include <algorithm>
#include <utility>

#include "dsl/ast/statements.hpp"
#include "dsl/diagnostics/diagnostics_engine.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/lowerer/detail/ast_lowerer.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::lowerer {

LowerResult::LowerResult(std::optional<ir::Program> program) : program_(std::move(program)) {}

bool LowerResult::ok() const { return program_.has_value(); }

const std::optional<ir::Program>& LowerResult::program() const { return program_; }

LowerResult lower(const semantic::AnalysisResult& analysis, DiagnosticsEngine& diagnostics) {
    const auto& [header, globals, tracks] = analysis.program();

    const auto first_lowering_diagnostic = diagnostics.diagnostics().size();
    ir::Program out;
    detail::lower_header(header, out);

    detail::LowererContext ctx(diagnostics);
    ctx.collect_patterns(globals);
    ctx.execute_block = [&ctx](const ast::Block& b, double& cur) { return detail::lower_block(b, ctx, cur); };

    detail::LowererScopeGuard scope(ctx);
    for (const auto& item : globals) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            if (const auto* let = std::get_if<ast::LetStatement>(&(*stmt_ptr)->kind)) {
                try {
                    detail::lower_let_statement(*let, ctx);
                } catch (const detail::LoweringFailure& error) {
                    ctx.report_lowering_error(error.what());
                }
            }
        }
    }

    for (const auto& track : tracks) {
        try {
            out.tracks.push_back(detail::lower_track_definition(track, ctx));
        } catch (const detail::LoweringFailure& error) {
            ctx.report_lowering_error(error.what());
        }
    }

    const auto& collected = diagnostics.diagnostics();
    const bool has_errors = std::ranges::any_of(collected.begin() + first_lowering_diagnostic,
                                                collected.end(),
                                                [](const Diagnostic& diagnostic) { return diagnostic.is_error(); });
    if (has_errors) {
        return LowerResult(std::nullopt);
    }

    return LowerResult(std::move(out));
}

}  // namespace dsl::lowerer
