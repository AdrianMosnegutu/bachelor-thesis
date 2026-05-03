#include "dsl/common/diagnostics/diagnostics_engine.hpp"

#include <algorithm>
#include <sstream>
#include <utility>

namespace dsl {

namespace {

std::string location_to_string(const source::Location& location) {
    std::ostringstream stream;
    stream << location;
    return stream.str();
}

}  // namespace

const char* to_string(const DiagnosticSeverity severity) {
    switch (severity) {
        case DiagnosticSeverity::Error:
            return "error";
        case DiagnosticSeverity::Warning:
            return "warning";
        case DiagnosticSeverity::Note:
            return "note";
    }

    return "unknown";
}

const char* to_string(const DiagnosticStage stage) {
    switch (stage) {
        case DiagnosticStage::Parsing:
            return "parse";
        case DiagnosticStage::Semantic:
            return "semantic";
        case DiagnosticStage::Lowering:
            return "lowering";
        case DiagnosticStage::Output:
            return "output";
    }

    return "unknown";
}

std::string format_diagnostic(const Diagnostic& diagnostic) {
    std::ostringstream stream;

    stream << to_string(diagnostic.severity) << ": " << to_string(diagnostic.stage) << ": ";
    if (diagnostic.location) {
        stream << *diagnostic.location << ": ";
    }
    stream << diagnostic.message;

    return stream.str();
}

void DiagnosticsEngine::report(Diagnostic diagnostic) { diagnostics_.push_back(std::move(diagnostic)); }

void DiagnosticsEngine::report(const DiagnosticStage stage,
                               const DiagnosticSeverity severity,
                               std::string message,
                               std::optional<std::string> location) {
    report(Diagnostic{
        .stage = stage,
        .severity = severity,
        .location = std::move(location),
        .message = std::move(message),
    });
}

void DiagnosticsEngine::report(const DiagnosticStage stage,
                               const DiagnosticSeverity severity,
                               const source::Location& location,
                               std::string message) {
    report(stage, severity, std::move(message), location_to_string(location));
}

const Diagnostics& DiagnosticsEngine::diagnostics() const { return diagnostics_; }

Diagnostics DiagnosticsEngine::take_diagnostics() { return std::move(diagnostics_); }

bool DiagnosticsEngine::has_errors() const {
    return std::ranges::any_of(diagnostics_, [](const Diagnostic& diagnostic) { return diagnostic.is_error(); });
}

bool DiagnosticsEngine::has_errors(const DiagnosticStage stage) const {
    return std::ranges::any_of(diagnostics_, [stage](const Diagnostic& diagnostic) {
        return diagnostic.stage == stage && diagnostic.is_error();
    });
}

}  // namespace dsl
