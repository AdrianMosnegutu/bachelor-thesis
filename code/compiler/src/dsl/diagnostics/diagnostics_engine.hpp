#pragma once

#include <string>

#include "dsl/diagnostics/diagnostic.hpp"
#include "dsl/source/location.hpp"

namespace dsl {

class DiagnosticsEngine {
   public:
    void report(Diagnostic diagnostic);
    void report(DiagnosticStage stage,
                DiagnosticSeverity severity,
                std::string message,
                std::optional<std::string> location = std::nullopt);
    void report(DiagnosticStage stage,
                DiagnosticSeverity severity,
                const source::Location& location,
                std::string message);

    [[nodiscard]] const Diagnostics& diagnostics() const;
    [[nodiscard]] Diagnostics take_diagnostics();

    [[nodiscard]] bool has_errors() const;
    [[nodiscard]] bool has_errors(DiagnosticStage stage) const;

   private:
    Diagnostics diagnostics_;
};

}  // namespace dsl
