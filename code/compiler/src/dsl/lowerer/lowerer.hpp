#pragma once

#include <optional>
#include <utility>

#include "dsl/diagnostics/diagnostics_engine.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/semantic/analysis_result.hpp"

namespace dsl::lowerer {

class LowerResult {
   public:
    explicit LowerResult(std::optional<ir::Program> program);

    [[nodiscard]] bool ok() const;
    [[nodiscard]] const std::optional<ir::Program>& program() const;

   private:
    std::optional<ir::Program> program_;
};

LowerResult lower(const semantic::AnalysisResult& analysis, DiagnosticsEngine& diagnostics);

}  // namespace dsl::lowerer
