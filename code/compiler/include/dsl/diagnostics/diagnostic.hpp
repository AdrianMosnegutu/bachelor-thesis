#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dsl {

enum class DiagnosticSeverity : std::uint8_t {
    Error,
    Warning,
    Note,
};

enum class DiagnosticStage : std::uint8_t {
    Parsing,
    Semantic,
    Lowering,
    Output,
};

struct Diagnostic {
    DiagnosticStage stage = DiagnosticStage::Parsing;
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::optional<std::string> location;
    std::string message;

    [[nodiscard]] bool is_error() const { return severity == DiagnosticSeverity::Error; }
};

using Diagnostics = std::vector<Diagnostic>;

[[nodiscard]] const char* to_string(DiagnosticSeverity severity);
[[nodiscard]] const char* to_string(DiagnosticStage stage);
[[nodiscard]] std::string format_diagnostic(const Diagnostic& diagnostic);

}  // namespace dsl
