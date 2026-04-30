#include "dsl/semantic/analysis_result.hpp"

#include <algorithm>

namespace dsl::semantic {

AnalysisResult::AnalysisResult(const ast::Program& program) : program_(&program) {}

bool AnalysisResult::has_errors() const {
    return std::ranges::any_of(diagnostics_, [](const Diagnostic& diagnostic) { return diagnostic.is_error(); });
}

}  // namespace dsl::semantic
