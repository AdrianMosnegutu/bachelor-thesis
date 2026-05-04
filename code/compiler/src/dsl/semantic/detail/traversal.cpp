#include "dsl/semantic/detail/traversal.hpp"

#include <algorithm>
#include <string>

#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/semantic/analysis_result.hpp"
#include "dsl/semantic/detail/scopes/scope_stack.hpp"

namespace dsl::semantic::detail {

Traversal::Traversal(AnalysisResult& result, DiagnosticsEngine& diagnostics)
    : result_(result), diagnostics_(diagnostics), scopes_(*result.symbols_) {}

void Traversal::run(const ast::Program& program) {
    ScopeStack::Guard guard(scopes_);

    validate_header(program.header);
    visit_globals(program.globals);
    for (const auto& track : program.tracks) {
        visit_track(track);
    }
}

void Traversal::validate_header(const ast::Header& header) const {
    if (header.tempo) {
        const int bpm = header.tempo->beats_per_minute;
        if (bpm < 1 || bpm > 1000) {
            diagnose(header.tempo->location, "tempo must be between 1 and 1000, got " + std::to_string(bpm));
        }
    }

    if (header.signature) {
        const int num = header.signature->beats;
        const int den = header.signature->unit;

        if (num < 1 || num > 32) {
            diagnose(header.signature->location,
                     "time signature numerator must be between 1 and 32, got " + std::to_string(num));
        }

        const bool is_power_of_two = den > 0 && (den & (den - 1)) == 0;
        if (den < 1 || den > 32 || !is_power_of_two) {
            diagnose(header.signature->location,
                     "time signature denominator must be a power of two between 1 and 32, got " +
                         std::to_string(den));
        }
    }
}

void Traversal::add_pattern_symbol(const ast::PatternDefinition& pattern) const {
    if (scopes_.find_in_current_scope_by_arity(pattern.name, pattern.params.size())) {
        diagnose(pattern.location,
                 "duplicate pattern '" + pattern.name + "' with " + std::to_string(pattern.params.size()) +
                     " parameter(s)");
        return;
    }
    scopes_.add_symbol(pattern.name, SymbolKind::Pattern, Type{TypeKind::Sequence}, pattern.location, &pattern);
}

bool Traversal::is_pattern_active(const ast::PatternDefinition& pattern) const {
    return std::ranges::find(active_patterns_, &pattern) != active_patterns_.end();
}

void Traversal::diagnose(const source::Location& location, std::string message) const {
    diagnostics_.report(DiagnosticStage::Semantic, DiagnosticSeverity::Error, location, std::move(message));
}

}  // namespace dsl::semantic::detail
