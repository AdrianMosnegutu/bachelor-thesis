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

void append_errors(CompileResult& result, CompileStage stage, const std::vector<std::string>& errors) {
    for (const auto& error : errors) {
        result.errors.push_back({stage, error});
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
            result.errors.push_back({CompileStage::Semantic, format_semantic_error(diagnostic)});
        }
    }
}

}  // namespace

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
        result.errors.push_back({CompileStage::Frontend, "input stream is null"});
        return result;
    }

    auto parse_result = frontend::parse_stream(input, source_name);
    append_errors(result, CompileStage::Frontend, parse_result.errors);
    if (!parse_result.ok()) {
        return result;
    }

    auto analysis = semantic::analyze(*parse_result.program);
    append_semantic_errors(result, analysis);
    if (!analysis.ok()) {
        return result;
    }

    try {
        auto program = ir::lower(analysis);
        backend::MidiWriter::write(program, output_path);
    } catch (const errors::LowererError& error) {
        result.errors.push_back({CompileStage::Lowering, error.format()});
    } catch (const std::exception& error) {
        result.errors.push_back({CompileStage::Backend, error.what()});
    }

    return result;
}

}  // namespace dsl
