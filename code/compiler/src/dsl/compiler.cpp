#include "dsl/compiler.hpp"

#include <algorithm>
#include <exception>
#include <utility>

#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/lowering/lower.hpp"
#include "dsl/midi/midi_writer.hpp"
#include "dsl/parsing/parse.hpp"
#include "dsl/semantic/analyze.hpp"

namespace dsl {

CompileResult::CompileResult(Diagnostics diagnostics) : diagnostics_(std::move(diagnostics)) {}

bool CompileResult::ok() const { return !has_errors(); }

bool CompileResult::has_errors() const {
    return std::ranges::any_of(diagnostics_, [](const Diagnostic& diagnostic) { return diagnostic.is_error(); });
}

void CompileResult::add_diagnostic(const Diagnostic& diagnostic) { diagnostics_.push_back(diagnostic); }

void CompileResult::add_diagnostic(const DiagnosticStage stage,
                                   const std::string& message,
                                   const DiagnosticSeverity severity) {
    diagnostics_.push_back(Diagnostic{
        .stage = stage,
        .severity = severity,
        .location = std::nullopt,
        .message = message,
    });
}

const Diagnostics& CompileResult::get_diagnostics() const { return diagnostics_; }

CompileResult compile(FILE* input, const std::string& source_name, const std::string& output_path) {
    DiagnosticsEngine diagnostics;

    if (input == nullptr) {
        diagnostics.report(DiagnosticStage::Parsing, DiagnosticSeverity::Error, "input stream is null");
        return CompileResult(diagnostics.take_diagnostics());
    }

    const auto parse_result = parsing::parse_stream(input, source_name, diagnostics);
    if (!parse_result.ok()) {
        return CompileResult(diagnostics.take_diagnostics());
    }

    const auto analysis = semantic::analyze(*parse_result.program(), diagnostics);
    if (diagnostics.has_errors(DiagnosticStage::Semantic)) {
        return CompileResult(diagnostics.take_diagnostics());
    }

    const auto lowered = lowering::lower(analysis, diagnostics);
    if (!lowered.ok()) {
        return CompileResult(diagnostics.take_diagnostics());
    }

    try {
        midi::MidiWriter::write(*lowered.program(), output_path);
    } catch (const std::exception& error) {
        diagnostics.report(DiagnosticStage::Output, DiagnosticSeverity::Error, error.what());
    }

    return CompileResult(diagnostics.take_diagnostics());
}

}  // namespace dsl
