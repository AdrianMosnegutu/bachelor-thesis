#pragma once

#include "dsl/semantic/analysis_result.hpp"

namespace dsl::ast {
struct Program;
}

namespace dsl::semantic {

[[nodiscard]] AnalysisResult analyze(const ast::Program& program);

}  // namespace dsl::semantic
