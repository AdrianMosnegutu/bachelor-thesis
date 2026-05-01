#include "dsl/semantic/analyzer.hpp"

#include "dsl/ast/program.hpp"
#include "dsl/semantic/detail/traversal.hpp"

namespace dsl::semantic {

AnalysisResult analyze(const ast::Program& program, DiagnosticsEngine& diagnostics) {
    AnalysisResult result(program);
    detail::Traversal(result, diagnostics).run(program);

    return result;
}

}  // namespace dsl::semantic
