#pragma once

#include "dsl/diagnostics/diagnostics_engine.hpp"
#include "dsl/lowerer/lower_result.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::lowerer {

LowerResult lower(const semantic::AnalysisResult& analysis, DiagnosticsEngine& diagnostics);

}  // namespace dsl::lowerer
