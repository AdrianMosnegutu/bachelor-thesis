#pragma once

#include <cstdio>
#include <string>

#include "dsl/diagnostics/diagnostic.hpp"

namespace dsl {

class CompileResult {
   public:
    CompileResult() = default;
    explicit CompileResult(Diagnostics diagnostics);

    [[nodiscard]] bool ok() const;
    [[nodiscard]] bool has_errors() const;

    void add_diagnostic(const Diagnostic& diagnostic);
    void add_diagnostic(DiagnosticStage stage,
                        const std::string& message,
                        DiagnosticSeverity severity = DiagnosticSeverity::Error);

    [[nodiscard]] const Diagnostics& get_diagnostics() const;

   private:
    Diagnostics diagnostics_;
};

[[nodiscard]] CompileResult compile(FILE* input, const std::string& source_name, const std::string& output_path);

}  // namespace dsl
