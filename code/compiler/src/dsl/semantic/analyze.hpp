#pragma once

#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::semantic {

[[nodiscard]] AnalysisResult analyze(const ast::Program& program, DiagnosticsEngine& diagnostics);

}  // namespace dsl::semantic
