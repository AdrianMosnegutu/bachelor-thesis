#include <gtest/gtest.h>

#include "dsl/semantic/type.hpp"
#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;
using dsl::semantic::TypeKind;

// -- Happy flows ---------------------------------------------------------------

TEST(TernaryTypeCheck, BoolConditionWithIntBranchesIsInt) {
    const auto [prog, result] = analyze_ok("let x = true ? 1 : 2;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Int);
}

TEST(TernaryTypeCheck, BoolConditionWithDoubleBranchesIsDouble) {
    const auto [prog, result] = analyze_ok("let x = false ? 1.5 : 2.5;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(TernaryTypeCheck, BoolConditionWithBoolBranchesIsBool) {
    const auto [prog, result] = analyze_ok("let x = true ? false : true;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Bool);
}

TEST(TernaryTypeCheck, BoolConditionWithNoteBranchesIsNote) {
    const auto [prog, result] = analyze_ok("let x = true ? A4 : B4;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Note);
}

TEST(TernaryTypeCheck, NoteInsidePlayViaTernary) {
    const auto [prog, result] = analyze_ok("track { play (true ? A4 : B4); }");
}

// -- Error cases ---------------------------------------------------------------

TEST(TernaryTypeCheck, IntConditionIsError) {
    const auto analyzed = analyze("track { play (1 ? A4 : B4); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "ternary condition"));
}

TEST(TernaryTypeCheck, NoteConditionIsError) {
    const auto analyzed = analyze("let x = A4 ? 1 : 2;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "ternary condition"));
}

TEST(TernaryTypeCheck, MismatchedBranchTypesIsError) {
    const auto analyzed = analyze("let x = true ? A4 : 3;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "ternary branches"));
}

TEST(TernaryTypeCheck, IntBranchVsDoubleBranchIsError) {
    // Int and Double are different types — branches must be identical
    const auto analyzed = analyze("let x = true ? 1 : 2.0;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "ternary branches"));
}

TEST(TernaryTypeCheck, ParenthesisedTernaryEvaluatingToIntIsInvalidPlayTarget) {
    const auto analyzed = analyze("track { play (true ? 1 : 2); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}
