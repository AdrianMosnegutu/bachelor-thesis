#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>

#include "dsl/common/ast/program.hpp"
#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/diagnostics/diagnostic.hpp"
#include "dsl/frontend/parse.hpp"
#include "dsl/lowerer/lower.hpp"
#include "dsl/semantic/analyze.hpp"

namespace {

struct LoweringInput {
    std::unique_ptr<dsl::ast::Program> program;
    dsl::semantic::AnalysisResult analysis;
    dsl::DiagnosticsEngine diagnostics;
};

LoweringInput analyze_for_lowering(const std::string& src) {
    dsl::DiagnosticsEngine diagnostics;
    auto parsed = dsl::frontend::parse_source(src, "<source>", diagnostics);
    EXPECT_TRUE(parsed.ok());
    auto program = parsed.take_program();
    auto analysis = dsl::semantic::analyze(*program, diagnostics);
    return {.program = std::move(program), .analysis = std::move(analysis), .diagnostics = std::move(diagnostics)};
}

bool has_lowering_error_containing(const dsl::Diagnostics& diagnostics, const std::string& text) {
    return std::ranges::any_of(diagnostics, [&](const dsl::Diagnostic& diagnostic) {
        return diagnostic.stage == dsl::DiagnosticStage::Lowering &&
               diagnostic.severity == dsl::DiagnosticSeverity::Error &&
               diagnostic.message.find(text) != std::string::npos;
    });
}

}  // namespace

TEST(LowererDiagnostics, ReportsMultipleLoweringDiagnosticsForSemanticallyValidProgram) {
    auto input = analyze_for_lowering(R"(
        track {
            loop (-1) { play A4; }
            loop (-2) { play B4; }
        }
    )");
    ASSERT_FALSE(input.diagnostics.has_errors(dsl::DiagnosticStage::Semantic));

    const auto result = dsl::lowerer::lower(input.analysis, input.diagnostics);

    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.program().has_value());
    EXPECT_TRUE(has_lowering_error_containing(input.diagnostics.diagnostics(), "loop count must be non-negative"));
    ASSERT_GE(input.diagnostics.diagnostics().size(), 2u);
    for (const auto& diagnostic : input.diagnostics.diagnostics()) {
        EXPECT_EQ(diagnostic.stage, dsl::DiagnosticStage::Lowering);
        EXPECT_EQ(diagnostic.severity, dsl::DiagnosticSeverity::Error);
    }
}

TEST(LowererDiagnostics, ReportsRuntimeExpressionFailuresAsLoweringDiagnostics) {
    auto input = analyze_for_lowering(R"(
        track {
            let first = 1 / 0;
            let second = 1 % 0;
        }
    )");
    ASSERT_FALSE(input.diagnostics.has_errors(dsl::DiagnosticStage::Semantic));

    const auto result = dsl::lowerer::lower(input.analysis, input.diagnostics);

    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.program().has_value());
    EXPECT_TRUE(has_lowering_error_containing(input.diagnostics.diagnostics(), "division by zero"));
    EXPECT_TRUE(has_lowering_error_containing(input.diagnostics.diagnostics(), "modulo by zero"));
}

TEST(LowererDiagnostics, SuccessfulLoweringReturnsProgram) {
    auto input = analyze_for_lowering("track { play A4; }");
    ASSERT_FALSE(input.diagnostics.has_errors(dsl::DiagnosticStage::Semantic));

    const auto result = dsl::lowerer::lower(input.analysis, input.diagnostics);

    ASSERT_TRUE(result.ok());
    ASSERT_TRUE(result.program().has_value());
    EXPECT_TRUE(input.diagnostics.diagnostics().empty());
    ASSERT_EQ(result.program()->tracks.size(), 1u);
    ASSERT_EQ(result.program()->tracks[0].events.size(), 1u);
}
