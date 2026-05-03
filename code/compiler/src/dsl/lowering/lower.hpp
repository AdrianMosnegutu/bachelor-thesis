#pragma once

#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/lowering/lower_result.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::lowering {

LowerResult lower(const semantic::AnalysisResult& analysis, DiagnosticsEngine& diagnostics);

}  // namespace dsl::lowering
