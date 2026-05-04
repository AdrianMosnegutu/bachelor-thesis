#include <gtest/gtest.h>

#include "dsl/semantic/type.hpp"
#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;
using dsl::semantic::TypeKind;

// -- Happy flows ---------------------------------------------------------------

TEST(UnaryTypeCheck, NegativeIntIsInt) {
    const auto [prog, result] = analyze_ok("let x = -(1);");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Int);
}

TEST(UnaryTypeCheck, NegativeDoubleIsDouble) {
    const auto [prog, result] = analyze_ok("let x = -(1.5);");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(UnaryTypeCheck, NotBoolIsBool) {
    const auto [prog, result] = analyze_ok("let x = !true;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Bool);
}

// -- Error cases ---------------------------------------------------------------

TEST(UnaryTypeCheck, NotIntIsError) {
    const auto analyzed = analyze("let x = !1;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "boolean"));
}

TEST(UnaryTypeCheck, NegativeBoolIsError) {
    const auto analyzed = analyze("let x = -true;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "numeric"));
}

TEST(UnaryTypeCheck, NegativeNoteIsError) {
    const auto analyzed = analyze("let x = -A4;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "numeric"));
}

TEST(UnaryTypeCheck, NotDoubleIsError) {
    const auto analyzed = analyze("let x = !1.5;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "boolean"));
}
