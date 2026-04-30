#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>

#include "dsl/core/ast/program.hpp"
#include "dsl/semantic/analyzer.hpp"
#include "dsl/semantic/type.hpp"
#include "parser.hpp"

struct yy_buffer_state;
using YY_BUFFER_STATE = yy_buffer_state*;

YY_BUFFER_STATE yy_scan_string(const char* str);
void yy_delete_buffer(YY_BUFFER_STATE buf);
void scanner_reset();

namespace {

struct ParseGuard {
    YY_BUFFER_STATE buf;

    explicit ParseGuard(const std::string& src) {
        scanner_reset();
        buf = yy_scan_string(src.c_str());
    }

    ~ParseGuard() {
        yy_delete_buffer(buf);
        scanner_reset();
    }

    ParseGuard(const ParseGuard&) = delete;
    ParseGuard& operator=(const ParseGuard&) = delete;
};

std::unique_ptr<dsl::ast::Program> parse(const std::string& src) {
    ParseGuard guard(src);
    auto program = std::make_unique<dsl::ast::Program>();
    dsl::Location loc;
    dsl::frontend::Parser parser{loc, *program};
    return parser.parse() == 0 ? std::move(program) : nullptr;
}

struct AnalyzedProgram {
    std::unique_ptr<dsl::ast::Program> program;
    dsl::semantic::AnalysisResult result;
};

AnalyzedProgram analyze(const std::string& src) {
    auto program = parse(src);
    EXPECT_NE(program, nullptr);
    auto result = dsl::semantic::analyze(*program);
    return AnalyzedProgram{std::move(program), std::move(result)};
}

bool has_error_containing(const dsl::semantic::AnalysisResult& result, const std::string& text) {
    return std::ranges::any_of(result.diagnostics(), [&](const dsl::semantic::Diagnostic& diagnostic) {
        return diagnostic.is_error() && diagnostic.message.find(text) != std::string::npos;
    });
}

}  // namespace

TEST(SemanticAnalyzer, EmptyProgramAnalyzesSuccessfully) {
    const auto analyzed = analyze("");
    const auto& result = analyzed.result;

    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.has_errors());
    EXPECT_TRUE(result.diagnostics().empty());
    EXPECT_EQ(result.symbols().scopes().size(), 1u);
}

TEST(SemanticAnalyzer, ValidProgramAnalyzesSuccessfully) {
    const auto analyzed = analyze(R"(
        let global_dur = 2;
        pattern riff(n) { play A4 :n; }
        track melody {
            let local_dur = global_dur + 1;
            play riff(local_dur);
            voice from local_dur { play C5; }
        }
    )");
    const auto& result = analyzed.result;

    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.diagnostics().empty());
    EXPECT_GE(result.symbols().symbols().size(), 4u);
    EXPECT_GT(result.annotations().expression_type_count(), 0u);
}

TEST(SemanticAnalyzer, ResultReferencesOriginalProgram) {
    const auto program = parse("track melody { play A4; }");
    ASSERT_NE(program, nullptr);

    const auto result = dsl::semantic::analyze(*program);

    EXPECT_EQ(&result.program(), program.get());
}

TEST(SemanticAnalyzer, AnnotatesLiteralExpressionTypes) {
    const auto program = parse("let answer = 42;");
    ASSERT_NE(program, nullptr);

    const auto result = dsl::semantic::analyze(*program);
    const auto& global = std::get<dsl::ast::StatementPtr>(program->globals[0]);
    const auto& let = std::get<dsl::ast::LetStatement>(global->kind);

    const auto type = result.annotations().expression_type(*let.value);
    ASSERT_TRUE(type.has_value());
    EXPECT_EQ(type->kind, dsl::semantic::TypeKind::Int);
}

TEST(SemanticAnalyzer, ReportsUndefinedVariable) {
    const auto analyzed = analyze("track { play missing; }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "undefined variable 'missing'"));
}

TEST(SemanticAnalyzer, ReportsAssignmentToUndeclaredVariable) {
    const auto analyzed = analyze("track { missing = 1; }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "assignment to undeclared variable 'missing'"));
}

TEST(SemanticAnalyzer, LetInitializerCannotReferenceOwnBinding) {
    const auto analyzed = analyze("track { let x = x; }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "undefined variable 'x'"));
}

TEST(SemanticAnalyzer, ReportsUndefinedPattern) {
    const auto analyzed = analyze("track { play missing(); }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "undefined pattern 'missing'"));
}

TEST(SemanticAnalyzer, ReportsPatternArityMismatch) {
    const auto analyzed = analyze(R"(
        pattern p(a) { play A4 :a; }
        track { play p(); }
    )");

    EXPECT_TRUE(has_error_containing(analyzed.result, "pattern 'p' expects 1 argument(s), got 0"));
}

TEST(SemanticAnalyzer, PatternCanBeCalledBeforeDeclarationInSameScope) {
    const auto analyzed = analyze("track { play p(); pattern p() { play A4; } }");

    EXPECT_TRUE(analyzed.result.ok());
}

TEST(SemanticAnalyzer, VoiceLocalPatternIsNotVisibleOutsideVoice) {
    const auto analyzed = analyze("track { voice { pattern p() { play A4; } } play p(); }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "undefined pattern 'p'"));
}

TEST(SemanticAnalyzer, VoiceLocalLetIsNotVisibleOutsideVoice) {
    const auto analyzed = analyze("track { voice { let x = 5; } play A4 :x; }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "undefined variable 'x'"));
}

TEST(SemanticAnalyzer, ForScopeIsVisibleToConditionBodyAndStep) {
    const auto analyzed = analyze("track { for (let i = 0; i < 2; i = i + 1) { play A4 :i; } }");

    EXPECT_TRUE(analyzed.result.ok());
}

TEST(SemanticAnalyzer, ReportsUnaryNumericOperandTypeError) {
    const auto analyzed = analyze("let x = -true;");

    EXPECT_TRUE(has_error_containing(analyzed.result, "unary '-' requires a numeric operand"));
}

TEST(SemanticAnalyzer, ReportsUnaryBooleanOperandTypeError) {
    const auto analyzed = analyze("let x = !1;");

    EXPECT_TRUE(has_error_containing(analyzed.result, "unary '!' requires a boolean operand"));
}

TEST(SemanticAnalyzer, ReportsBinaryNumericOperandTypeError) {
    const auto analyzed = analyze("let x = true + 1;");

    EXPECT_TRUE(has_error_containing(analyzed.result, "left operand must be numeric"));
}

TEST(SemanticAnalyzer, ReportsModuloIntegerOperandTypeError) {
    const auto analyzed = analyze("let x = 1.5 % 1;");

    EXPECT_TRUE(has_error_containing(analyzed.result, "modulo requires integer operands"));
}

TEST(SemanticAnalyzer, ReportsEqualityOperandTypeError) {
    const auto analyzed = analyze("let x = A4 == true;");

    EXPECT_TRUE(has_error_containing(analyzed.result, "'==' requires numeric or boolean operands"));
}

TEST(SemanticAnalyzer, ReportsLogicalOperandTypeError) {
    const auto analyzed = analyze("let x = 1 && true;");

    EXPECT_TRUE(has_error_containing(analyzed.result, "'&&' requires boolean operands"));
}

TEST(SemanticAnalyzer, ReportsTernaryConditionTypeError) {
    const auto analyzed = analyze("track { play (1 ? A4 : B4); }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "ternary condition must be a boolean expression"));
}

TEST(SemanticAnalyzer, ReportsTernaryBranchTypeError) {
    const auto analyzed = analyze("let x = true ? A4 : 3;");

    EXPECT_TRUE(has_error_containing(analyzed.result, "ternary branches must evaluate to the same type"));
}

TEST(SemanticAnalyzer, ReportsSequenceDurationTypeError) {
    const auto analyzed = analyze("track { play [A4 :true]; }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "sequence item duration must be numeric"));
}

TEST(SemanticAnalyzer, ReportsChordMemberTypeError) {
    const auto analyzed = analyze("track { let n = 1; play (n, C5); }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "chord members must be notes"));
}

TEST(SemanticAnalyzer, ReportsChordDurationTypeError) {
    const auto analyzed = analyze("track { play (A4 :true, C5); }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "chord note duration must be numeric"));
}

TEST(SemanticAnalyzer, ReportsPlayDurationTypeError) {
    const auto analyzed = analyze("track { play A4 :true; }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "play duration must be a number"));
}

TEST(SemanticAnalyzer, ReportsFromOffsetTypeError) {
    const auto analyzed = analyze("track { play A4 from true; }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "from offset must be a number"));
}

TEST(SemanticAnalyzer, ReportsVoiceFromTypeError) {
    const auto analyzed = analyze("track { voice from true { play A4; } }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "voice 'from' expression must be numeric"));
}

TEST(SemanticAnalyzer, ReportsIfConditionTypeError) {
    const auto analyzed = analyze("track { if (1) { play A4; } }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "if condition must be a boolean"));
}

TEST(SemanticAnalyzer, ReportsForConditionTypeError) {
    const auto analyzed = analyze("track { for (let i = 0; i + 1; i = i + 1) { play A4; } }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "for condition must be a boolean"));
}

TEST(SemanticAnalyzer, ReportsLoopCountTypeError) {
    const auto analyzed = analyze("track { loop (1.5) { play A4; } }");

    EXPECT_TRUE(has_error_containing(analyzed.result, "loop count must be an integer"));
}

TEST(SemanticAnalyzer, ReportsPatternArgumentTypeErrorInInstantiatedBody) {
    const auto analyzed = analyze(R"(
        pattern p(n) { play A4 :n; }
        track { play p(true); }
    )");

    EXPECT_TRUE(has_error_containing(analyzed.result, "play duration must be a number"));
}

TEST(SemanticAnalyzer, AnnotatesBinaryExpressionResultType) {
    const auto program = parse("let mixed = 1 + 1.5;");
    ASSERT_NE(program, nullptr);

    const auto result = dsl::semantic::analyze(*program);
    const auto& global = std::get<dsl::ast::StatementPtr>(program->globals[0]);
    const auto& let = std::get<dsl::ast::LetStatement>(global->kind);

    const auto type = result.annotations().expression_type(*let.value);
    ASSERT_TRUE(type.has_value());
    EXPECT_EQ(type->kind, dsl::semantic::TypeKind::Double);
}
