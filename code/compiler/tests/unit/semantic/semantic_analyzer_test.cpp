#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>

#include "dsl/common/ast/program.hpp"
#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/parsing/parse.hpp"
#include "dsl/semantic/analyze.hpp"
#include "dsl/semantic/type.hpp"

namespace {

std::unique_ptr<dsl::ast::Program> parse(const std::string& src) {
    dsl::DiagnosticsEngine diagnostics;
    return dsl::parsing::parse_source(src, "<source>", diagnostics).take_program();
}

std::unique_ptr<dsl::ast::Program> parse(const std::string& src, dsl::DiagnosticsEngine& diagnostics) {
    return dsl::parsing::parse_source(src, "<source>", diagnostics).take_program();
}

struct AnalyzedProgram {
    std::unique_ptr<dsl::ast::Program> program;
    dsl::semantic::AnalysisResult result;
    dsl::Diagnostics diagnostics;
};

AnalyzedProgram analyze(const std::string& src) {
    dsl::DiagnosticsEngine diagnostics;
    auto program = parse(src, diagnostics);
    EXPECT_NE(program, nullptr);
    auto result = dsl::semantic::analyze(*program, diagnostics);
    return AnalyzedProgram{std::move(program), std::move(result), diagnostics.take_diagnostics()};
}

bool has_error_containing(const dsl::Diagnostics& diagnostics, const std::string& text) {
    return std::ranges::any_of(diagnostics, [&](const dsl::Diagnostic& diagnostic) {
        return diagnostic.severity == dsl::DiagnosticSeverity::Error &&
               diagnostic.message.find(text) != std::string::npos;
    });
}

bool has_semantic_errors(const dsl::Diagnostics& diagnostics) {
    return std::ranges::any_of(diagnostics, [](const dsl::Diagnostic& diagnostic) {
        return diagnostic.stage == dsl::DiagnosticStage::Semantic && diagnostic.is_error();
    });
}

}  // namespace

TEST(SemanticAnalyzer, EmptyProgramAnalyzesSuccessfully) {
    const auto [program, result, diagnostics] = analyze("");

    EXPECT_FALSE(has_semantic_errors(diagnostics));
    EXPECT_TRUE(diagnostics.empty());
    EXPECT_FALSE(has_semantic_errors(diagnostics));
}

TEST(SemanticAnalyzer, ValidProgramAnalyzesSuccessfully) {
    const auto [program, result, diagnostics] = analyze(R"(
        let global_dur = 2;
        pattern riff(n) { play A4 :n; }
        track melody {
            let local_dur = global_dur + 1;
            play riff(local_dur);
            voice from local_dur { play C5; }
        }
    )");

    EXPECT_FALSE(has_semantic_errors(diagnostics));
    EXPECT_TRUE(diagnostics.empty());
    EXPECT_FALSE(has_semantic_errors(diagnostics));
}

TEST(SemanticAnalyzer, ResultReferencesOriginalProgram) {
    dsl::DiagnosticsEngine diagnostics;
    const auto program = parse("track melody { play A4; }", diagnostics);
    ASSERT_NE(program, nullptr);

    const auto result = dsl::semantic::analyze(*program, diagnostics);

    EXPECT_EQ(&result.program(), program.get());
}

TEST(SemanticAnalyzer, AnnotatesLiteralExpressionTypes) {
    dsl::DiagnosticsEngine diagnostics;
    const auto program = parse("let answer = 42;", diagnostics);
    ASSERT_NE(program, nullptr);

    const auto result = dsl::semantic::analyze(*program, diagnostics);
    const auto& global = std::get<dsl::ast::StatementPtr>(program->globals[0]);
    const auto& let = std::get<dsl::ast::LetStatement>(global->kind);

    const auto type = result.get_expression_type(*let.value);
    ASSERT_TRUE(type.has_value());
    EXPECT_EQ(type->kind, dsl::semantic::TypeKind::Int);
}

TEST(SemanticAnalyzer, ReportsUndefinedVariable) {
    const auto analyzed = analyze("track { play missing; }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "undefined variable 'missing'"));
}

TEST(SemanticAnalyzer, ReportsAssignmentToUndeclaredVariable) {
    const auto analyzed = analyze("track { missing = 1; }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "assignment to undeclared variable 'missing'"));
}

TEST(SemanticAnalyzerDiagnostics, UsesSharedDiagnosticsAndAccumulatesMultipleErrors) {
    const auto analyzed = analyze(R"(
        track {
            play missing;
            play also_missing;
            if (1) { play A4; }
        }
    )");

    ASSERT_GE(analyzed.diagnostics.size(), 3u);
    for (const auto& diagnostic : analyzed.diagnostics) {
        EXPECT_EQ(diagnostic.stage, dsl::DiagnosticStage::Semantic);
        EXPECT_EQ(diagnostic.severity, dsl::DiagnosticSeverity::Error);
    }
    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "undefined variable 'missing'"));
    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "undefined variable 'also_missing'"));
    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "if condition must be a boolean"));
}

TEST(SemanticAnalyzer, LetInitializerCannotReferenceOwnBinding) {
    const auto analyzed = analyze("track { let x = x; }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "undefined variable 'x'"));
}

TEST(SemanticAnalyzer, ReportsUndefinedPattern) {
    const auto analyzed = analyze("track { play missing(); }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "undefined pattern 'missing'"));
}

TEST(SemanticAnalyzer, ReportsPatternArityMismatch) {
    const auto analyzed = analyze(R"(
        pattern p(a) { play A4 :a; }
        track { play p(); }
    )");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "pattern 'p' expects 1 argument(s), got 0"));
}

TEST(SemanticAnalyzer, PatternCanBeCalledBeforeDeclarationInSameScope) {
    const auto analyzed = analyze("track { play p(); pattern p() { play A4; } }");

    EXPECT_FALSE(has_semantic_errors(analyzed.diagnostics));
}

TEST(SemanticAnalyzer, VoiceLocalPatternIsNotVisibleOutsideVoice) {
    const auto analyzed = analyze("track { voice { pattern p() { play A4; } } play p(); }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "undefined pattern 'p'"));
}

TEST(SemanticAnalyzer, VoiceLocalLetIsNotVisibleOutsideVoice) {
    const auto analyzed = analyze("track { voice { let x = 5; } play A4 :x; }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "undefined variable 'x'"));
}

TEST(SemanticAnalyzer, ForScopeIsVisibleToConditionBodyAndStep) {
    const auto analyzed = analyze("track { for (let i = 0; i < 2; i = i + 1) { play A4 :i; } }");

    EXPECT_FALSE(has_semantic_errors(analyzed.diagnostics));
}

TEST(SemanticAnalyzer, ReportsUnaryNumericOperandTypeError) {
    const auto analyzed = analyze("let x = -true;");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "unary '-' requires a numeric operand"));
}

TEST(SemanticAnalyzer, ReportsUnaryBooleanOperandTypeError) {
    const auto analyzed = analyze("let x = !1;");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "unary '!' requires a boolean operand"));
}

TEST(SemanticAnalyzer, ReportsBinaryNumericOperandTypeError) {
    const auto analyzed = analyze("let x = true + 1;");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "left operand must be numeric"));
}

TEST(SemanticAnalyzer, ReportsModuloIntegerOperandTypeError) {
    const auto analyzed = analyze("let x = 1.5 % 1;");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "modulo requires integer operands"));
}

TEST(SemanticAnalyzer, ReportsEqualityOperandTypeError) {
    const auto analyzed = analyze("let x = A4 == true;");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "'==' requires numeric or boolean operands"));
}

TEST(SemanticAnalyzer, ReportsLogicalOperandTypeError) {
    const auto analyzed = analyze("let x = 1 && true;");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "'&&' requires boolean operands"));
}

TEST(SemanticAnalyzer, ReportsTernaryConditionTypeError) {
    const auto analyzed = analyze("track { play (1 ? A4 : B4); }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "ternary condition must be a boolean expression"));
}

TEST(SemanticAnalyzer, ReportsTernaryBranchTypeError) {
    const auto analyzed = analyze("let x = true ? A4 : 3;");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "ternary branches must evaluate to the same type"));
}

TEST(SemanticAnalyzer, ReportsSequenceDurationTypeError) {
    const auto analyzed = analyze("track { play [A4 :true]; }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "sequence item duration must be numeric"));
}

TEST(SemanticAnalyzer, ReportsChordMemberTypeError) {
    const auto analyzed = analyze("track { let n = 1; play (n, C5); }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "chord members must be notes"));
}

TEST(SemanticAnalyzer, ReportsChordDurationTypeError) {
    const auto analyzed = analyze("track { play (A4 :true, C5); }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "chord note duration must be numeric"));
}

TEST(SemanticAnalyzer, ReportsPlayDurationTypeError) {
    const auto analyzed = analyze("track { play A4 :true; }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "play duration must be a number"));
}

TEST(SemanticAnalyzer, ReportsFromOffsetTypeError) {
    const auto analyzed = analyze("track { play A4 from true; }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "from offset must be a number"));
}

TEST(SemanticAnalyzer, ReportsVoiceFromTypeError) {
    const auto analyzed = analyze("track { voice from true { play A4; } }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "voice 'from' expression must be numeric"));
}

TEST(SemanticAnalyzer, ReportsIfConditionTypeError) {
    const auto analyzed = analyze("track { if (1) { play A4; } }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "if condition must be a boolean"));
}

TEST(SemanticAnalyzer, ReportsForConditionTypeError) {
    const auto analyzed = analyze("track { for (let i = 0; i + 1; i = i + 1) { play A4; } }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "for condition must be a boolean"));
}

TEST(SemanticAnalyzer, ReportsLoopCountTypeError) {
    const auto analyzed = analyze("track { loop (1.5) { play A4; } }");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "loop count must be an integer"));
}

TEST(SemanticAnalyzer, ReportsPatternArgumentTypeErrorInInstantiatedBody) {
    const auto analyzed = analyze(R"(
        pattern p(n) { play A4 :n; }
        track { play p(true); }
    )");

    EXPECT_TRUE(has_error_containing(analyzed.diagnostics, "play duration must be a number"));
}

TEST(SemanticAnalyzer, AnnotatesBinaryExpressionResultType) {
    dsl::DiagnosticsEngine diagnostics;
    const auto program = parse("let mixed = 1 + 1.5;", diagnostics);
    ASSERT_NE(program, nullptr);

    const auto result = dsl::semantic::analyze(*program, diagnostics);
    const auto& global = std::get<dsl::ast::StatementPtr>(program->globals[0]);
    const auto& let = std::get<dsl::ast::LetStatement>(global->kind);

    const auto type = result.get_expression_type(*let.value);
    ASSERT_TRUE(type.has_value());
    EXPECT_EQ(type->kind, dsl::semantic::TypeKind::Double);
}
