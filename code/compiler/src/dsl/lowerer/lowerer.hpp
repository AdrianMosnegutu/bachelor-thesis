#pragma once

#include "dsl/ir/program.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::lowerer {

ir::Program lower(const semantic::AnalysisResult& analysis);

}  // namespace dsl::lowerer
