#include <gtest/gtest.h>

#include "dsl/semantic/type.hpp"
#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;
using dsl::semantic::TypeKind;

// -- Basic let declarations ----------------------------------------------------

TEST(LetDeclaration, IntLiteralInfersIntType) {
    const auto [prog, result] = analyze_ok("let x = 42;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Int);
}

TEST(LetDeclaration, DoubleLiteralInfersDoubleType) {
    const auto [prog, result] = analyze_ok("let x = 1.5;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(LetDeclaration, BoolLiteralInfersBoolType) {
    const auto [prog, result] = analyze_ok("let x = true;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Bool);
}

TEST(LetDeclaration, NoteLiteralInfersNoteType) {
    const auto [prog, result] = analyze_ok("let x = A4;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Note);
}

// -- Let with complex expressions ----------------------------------------------

TEST(LetDeclaration, LetBoundToArithmeticExpressionIsValid) {
    const auto [prog, result] = analyze_ok("let x = 1 + 2;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Int);
}

TEST(LetDeclaration, LetBoundToAnotherIdentifierIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        let a = 5;
        let b = a;
    )");
}

TEST(LetDeclaration, LetBoundToChordIsValid) {
    const auto [prog, result] = analyze_ok("let c = (A4, C5);");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Chord);
}

TEST(LetDeclaration, LetBoundToSequenceIsValid) {
    const auto [prog, result] = analyze_ok("let s = [A4, B4];");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Sequence);
}

TEST(LetDeclaration, LetBoundToTernaryIsValid) {
    const auto [prog, result] = analyze_ok("let x = true ? 1.3 : 2.4;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

// -- Let re-declaration --------------------------------------------------------

TEST(LetDeclaration, RedeclarationInSameScopeIsError) {
    const auto analyzed = analyze(R"(
        let x = 1;
        let x = 2;
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(LetDeclaration, RedeclarationInSameTrackScopeIsError) {
    const auto analyzed = analyze(R"(
        track {
            let x = 1;
            let x = 2;
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(LetDeclaration, RedeclarationInSamePatternScopeIsError) {
    const auto analyzed = analyze(R"(
        track {
            pattern foo() {
                let x = 1;
                let x = 2;
            }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(LetDeclaration, RedeclarationInSameVoiceScopeIsError) {
    const auto analyzed = analyze(R"(
        track {
            voice {
                let x = 1;
                let x = 2;
            }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}
