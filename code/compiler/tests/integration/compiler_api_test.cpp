#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <string>

#include "dsl/compiler.hpp"

namespace {

using FilePtr = std::unique_ptr<FILE, int (*)(FILE*)>;
namespace fs = std::filesystem;

fs::path test_path(const std::string& filename) { return fs::temp_directory_path() / filename; }

void write_text_file(const fs::path& path, const std::string& contents) {
    std::ofstream file(path);
    file << contents;
}

dsl::CompileResult compile_source_file(const std::string& filename,
                                       const std::string& source,
                                       const fs::path& output_path) {
    const fs::path source_path = test_path(filename);
    write_text_file(source_path, source);

    const FilePtr input(std::fopen(source_path.string().c_str(), "r"), &std::fclose);
    EXPECT_NE(input, nullptr);

    auto result = dsl::compile(input.get(), source_path.string(), output_path.string());
    fs::remove(source_path);
    return result;
}

bool has_diagnostic(const dsl::CompileResult& result,
                    const dsl::DiagnosticStage stage,
                    const dsl::DiagnosticSeverity severity) {
    for (const auto& diagnostic : result.get_diagnostics()) {
        if (diagnostic.stage == stage && diagnostic.severity == severity) {
            return true;
        }
    }
    return false;
}

}  // namespace

TEST(CompilerApi, CompilesSourceFileToMidi) {
    const fs::path source_path = test_path("dsl_compiler_api_valid.dsl");
    const fs::path output_path = test_path("dsl_compiler_api_valid.mid");
    fs::remove(output_path);
    write_text_file(source_path, "track { play A4; }");

    const FilePtr input(std::fopen(source_path.string().c_str(), "r"), &std::fclose);
    ASSERT_NE(input, nullptr);

    const auto result = dsl::compile(input.get(), source_path.string(), output_path.string());

    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.get_diagnostics().empty());
    ASSERT_TRUE(fs::exists(output_path));
    EXPECT_GT(fs::file_size(output_path), 0u);

    fs::remove(source_path);
    fs::remove(output_path);
}

TEST(CompilerApi, ReportsSemanticErrorsWithoutWritingMidi) {
    const fs::path source_path = test_path("dsl_compiler_api_invalid.dsl");
    const fs::path output_path = test_path("dsl_compiler_api_invalid.mid");
    fs::remove(output_path);
    write_text_file(source_path, "track { play missing; }");

    const FilePtr input(std::fopen(source_path.string().c_str(), "r"), &std::fclose);
    ASSERT_NE(input, nullptr);

    const auto result = dsl::compile(input.get(), source_path.string(), output_path.string());

    ASSERT_FALSE(result.ok());
    ASSERT_FALSE(result.get_diagnostics().empty());
    EXPECT_EQ(result.get_diagnostics().front().stage, dsl::DiagnosticStage::Semantic);
    EXPECT_FALSE(fs::exists(output_path));

    fs::remove(source_path);
}

TEST(CompilerApiDiagnostics, ExposesSeverityAndStageNames) {
    EXPECT_STREQ(dsl::to_string(dsl::DiagnosticSeverity::Error), "error");
    EXPECT_STREQ(dsl::to_string(dsl::DiagnosticSeverity::Warning), "warning");
    EXPECT_STREQ(dsl::to_string(dsl::DiagnosticSeverity::Note), "note");

    EXPECT_STREQ(dsl::to_string(dsl::DiagnosticStage::Lexical), "lexical");
    EXPECT_STREQ(dsl::to_string(dsl::DiagnosticStage::Syntax), "syntax");
    EXPECT_STREQ(dsl::to_string(dsl::DiagnosticStage::Semantic), "semantic");
    EXPECT_STREQ(dsl::to_string(dsl::DiagnosticStage::Lowering), "lowering");
    EXPECT_STREQ(dsl::to_string(dsl::DiagnosticStage::Output), "output");
}

TEST(CompilerApiDiagnostics, DistinguishesLexicalAndSyntaxStages) {
    const fs::path output_path = test_path("dsl_diagnostics_frontend.mid");
    fs::remove(output_path);

    const auto lexical_result = compile_source_file("dsl_diagnostics_lexical.dsl", "track { play @; }", output_path);
    EXPECT_FALSE(lexical_result.ok());
    EXPECT_TRUE(has_diagnostic(lexical_result, dsl::DiagnosticStage::Lexical, dsl::DiagnosticSeverity::Error));

    const auto syntax_result = compile_source_file("dsl_diagnostics_syntax.dsl", "track { play ; }", output_path);
    EXPECT_FALSE(syntax_result.ok());
    EXPECT_TRUE(has_diagnostic(syntax_result, dsl::DiagnosticStage::Syntax, dsl::DiagnosticSeverity::Error));

    fs::remove(output_path);
}

TEST(CompilerApiDiagnostics, StopsBeforeOutputWhenSemanticErrorsExist) {
    const fs::path output_path = test_path("dsl_diagnostics_semantic_gate.mid");
    fs::remove(output_path);

    const auto result = compile_source_file("dsl_diagnostics_semantic_gate.dsl",
                                            "track { play missing; play alsomissing; }",
                                            output_path);

    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_diagnostic(result, dsl::DiagnosticStage::Semantic, dsl::DiagnosticSeverity::Error));
    EXPECT_FALSE(has_diagnostic(result, dsl::DiagnosticStage::Lowering, dsl::DiagnosticSeverity::Error));
    EXPECT_FALSE(has_diagnostic(result, dsl::DiagnosticStage::Output, dsl::DiagnosticSeverity::Error));
    EXPECT_FALSE(fs::exists(output_path));
}

TEST(CompilerApiDiagnostics, ReportsLoweringErrorsWithLoweringStage) {
    const fs::path output_path = test_path("dsl_diagnostics_lowering_gate.mid");
    fs::remove(output_path);

    const auto result =
        compile_source_file("dsl_diagnostics_lowering_gate.dsl", "track { loop (-1) { play A4; } }", output_path);

    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_diagnostic(result, dsl::DiagnosticStage::Lowering, dsl::DiagnosticSeverity::Error));
    EXPECT_FALSE(has_diagnostic(result, dsl::DiagnosticStage::Semantic, dsl::DiagnosticSeverity::Error));
    EXPECT_FALSE(has_diagnostic(result, dsl::DiagnosticStage::Output, dsl::DiagnosticSeverity::Error));
    EXPECT_FALSE(fs::exists(output_path));
}

TEST(CompilerApiDiagnostics, ReportsOutputFailuresWithOutputStage) {
    const fs::path output_dir = test_path("dsl_diagnostics_missing_output_directory");
    fs::remove_all(output_dir);
    const fs::path output_path = output_dir / "out.mid";

    const auto result = compile_source_file("dsl_diagnostics_output.dsl", "track { play A4; }", output_path);

    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_diagnostic(result, dsl::DiagnosticStage::Output, dsl::DiagnosticSeverity::Error));
    fs::remove_all(output_dir);
}
