#include "dsl/semantic/detail/traversal.hpp"

#include <algorithm>

#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/semantic/analysis_result.hpp"
#include "dsl/semantic/detail/scopes/scope_stack.hpp"

namespace dsl::semantic::detail {

Traversal::Traversal(AnalysisResult& result, DiagnosticsEngine& diagnostics)
    : result_(result), diagnostics_(diagnostics), scopes_(*result.symbols_) {}

void Traversal::run(const ast::Program& program) {
    ScopeStack::Guard guard(scopes_);

    visit_globals(program.globals);
    for (const auto& track : program.tracks) {
        visit_track(track);
    }
}

void Traversal::add_pattern_symbol(const ast::PatternDefinition& pattern) const {
    scopes_.add_symbol(pattern.name, SymbolKind::Pattern, Type{TypeKind::Sequence}, pattern.location, &pattern);
}

bool Traversal::is_pattern_active(const ast::PatternDefinition& pattern) const {
    return std::ranges::find(active_patterns_, &pattern) != active_patterns_.end();
}

void Traversal::diagnose(const source::Location& location, std::string message) const {
    diagnostics_.report(DiagnosticStage::Semantic, DiagnosticSeverity::Error, location, std::move(message));
}

}  // namespace dsl::semantic::detail
