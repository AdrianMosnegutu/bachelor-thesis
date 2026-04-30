#pragma once

#include "dsl/core/ast/program.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::semantic {

[[nodiscard]] AnalysisResult analyze(const ast::Program& program);

}  // namespace dsl::semantic
