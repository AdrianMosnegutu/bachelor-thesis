#include <gtest/gtest.h>

#include "dsl/semantic/type.hpp"
#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;
using dsl::semantic::TypeKind;

// -- Happy flows ---------------------------------------------------------------

TEST(LogicalTypeCheck, BoolAndBoolIsBool) {
    const auto [prog, result] = analyze_ok("let x = true && false;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Bool);
}

TEST(LogicalTypeCheck, BoolOrBoolIsBool) {
    const auto [prog, result] = analyze_ok("let x = true || false;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Bool);
}

// -- Error cases ---------------------------------------------------------------

TEST(LogicalTypeCheck, IntAndBoolIsError) {
    const auto analyzed = analyze("let x = 1 && true;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "'&&'"));
}

TEST(LogicalTypeCheck, BoolAndIntIsError) {
    const auto analyzed = analyze("let x = true && 1;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "'&&'"));
}

TEST(LogicalTypeCheck, IntOrIntIsError) {
    const auto analyzed = analyze("let x = 1 || 2;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "boolean"));
}

TEST(LogicalTypeCheck, BoolOrDoubleIsError) {
    const auto analyzed = analyze("let x = true || 1.5;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "boolean"));
}

TEST(LogicalTypeCheck, NoteAndBoolIsError) {
    const auto analyzed = analyze("let x = A4 && true;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "'&&'"));
}
