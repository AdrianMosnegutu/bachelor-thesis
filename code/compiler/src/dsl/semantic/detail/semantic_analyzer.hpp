#pragma once

#include "dsl/ast/program.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::semantic::detail {

class SemanticAnalyzer {
   public:
    explicit SemanticAnalyzer(const ast::Program& program);

    [[nodiscard]] AnalysisResult analyze() const;

   private:
    const ast::Program& program_;
};

}  // namespace dsl::semantic::detail
