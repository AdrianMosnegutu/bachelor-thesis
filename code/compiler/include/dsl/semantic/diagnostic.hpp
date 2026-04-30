#pragma once

#include <string>

#include "dsl/core/location.hpp"

namespace dsl::semantic {

enum class DiagnosticSeverity {
    Note,
    Warning,
    Error,
};

struct Diagnostic {
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    Location location;
    std::string message;

    [[nodiscard]] bool is_error() const { return severity == DiagnosticSeverity::Error; }
};

}  // namespace dsl::semantic
