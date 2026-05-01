#include "dsl/lowerer/lowerer.hpp"

#include <algorithm>
#include <utility>

#include "dsl/ast/statements.hpp"
#include "dsl/diagnostics/diagnostics_engine.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/lowerer/detail/ast_lowerer.hpp"
#include "dsl/lowerer/lower_result.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::lowerer {

LowerResult lower(const semantic::AnalysisResult& analysis, DiagnosticsEngine& diagnostics) {
    const auto& [header, globals, tracks] = analysis.program();

    const auto first_lowering_diagnostic = diagnostics.diagnostics().size();
    ir::Program out;
    detail::lower_header(header, out);

    detail::LowererContext context(diagnostics);
    context.collect_patterns(globals);
    context.execute_block = [&context](const ast::Block& block, double& current) {
        return detail::lower_block(block, context, current);
    };

    detail::LowererScopeGuard scope(context);
    for (const auto& item : globals) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            if (const auto* let = std::get_if<ast::LetStatement>(&(*stmt_ptr)->kind)) {
                try {
                    detail::lower_let_statement(*let, context);
                } catch (const detail::LoweringFailure& error) {
                    context.report_lowering_error(error.what());
                }
            }
        }
    }

    for (const auto& track : tracks) {
        try {
            out.tracks.push_back(detail::lower_track_definition(track, context));
        } catch (const detail::LoweringFailure& error) {
            context.report_lowering_error(error.what());
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
