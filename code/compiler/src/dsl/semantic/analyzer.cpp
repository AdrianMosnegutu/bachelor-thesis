#include "dsl/semantic/analyzer.hpp"

#include "dsl/semantic/detail/semantic_analyzer.hpp"

namespace dsl::semantic {

AnalysisResult analyze(const ast::Program& program) { return detail::SemanticAnalyzer(program).analyze(); }

}  // namespace dsl::semantic
