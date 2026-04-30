#include "dsl/compiler.hpp"

#include <exception>
#include <sstream>

#include "dsl/backend/midi_writer.hpp"
#include "dsl/errors/lowerer_error.hpp"
#include "dsl/frontend/parse.hpp"
#include "dsl/ir/lowerer.hpp"
#include "dsl/semantic/analyzer.hpp"

namespace dsl {

namespace {

void append_errors(CompileResult& result, const CompileStage stage, const std::vector<std::string>& errors) {
    for (const auto& error : errors) {
        result.add_diagnostic(stage, error);
    }
}

std::string format_semantic_error(const semantic::Diagnostic& diagnostic) {
    std::ostringstream stream;
    stream << diagnostic.location << ": " << diagnostic.message;
    return stream.str();
}

void append_semantic_errors(CompileResult& result, const semantic::AnalysisResult& analysis) {
    for (const auto& diagnostic : analysis.diagnostics()) {
        if (diagnostic.is_error()) {
            result.add_diagnostic(CompileStage::Semantic, format_semantic_error(diagnostic));
        }
    }
}

}  // namespace

bool CompileResult::ok() const { return diagnostics_.empty(); }

void CompileResult::add_diagnostic(const Diagnostic& diagnostic) { diagnostics_.push_back(diagnostic); }

void CompileResult::add_diagnostic(CompileStage stage, const std::string& message) {
    diagnostics_.emplace_back(stage, message);
}

const Diagnostics& CompileResult::get_diagnostics() const { return diagnostics_; }

const char* to_string(const CompileStage stage) {
    switch (stage) {
        case CompileStage::Frontend:
            return "frontend";
        case CompileStage::Semantic:
            return "semantic";
        case CompileStage::Lowering:
            return "lowering";
        case CompileStage::Backend:
            return "backend";
    }

    return "unknown";
}

CompileResult compile(FILE* input, const std::string& source_name, const std::string& output_path) {
    CompileResult result;

    if (input == nullptr) {
        result.add_diagnostic(CompileStage::Frontend, "input stream is null");
        return result;
    }

    const auto parse_result = frontend::parse_stream(input, source_name);
    append_errors(result, CompileStage::Frontend, parse_result.errors);
    if (!parse_result.ok()) {
        return result;
    }

    const auto analysis = semantic::analyze(*parse_result.program);
    append_semantic_errors(result, analysis);
    if (!analysis.ok()) {
        return result;
    }

    try {
        const auto program = ir::lower(analysis);
        backend::MidiWriter::write(program, output_path);
    } catch (const errors::LowererError& error) {
        result.add_diagnostic(CompileStage::Lowering, error.format());
    } catch (const std::exception& error) {
        result.add_diagnostic(CompileStage::Backend, error.what());
    }

    return result;
}

}  // namespace dsl
