#include <gtest/gtest.h>

#include "dsl/semantic/type.hpp"
#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;
using dsl::semantic::TypeKind;

// -- Happy flows: result type annotation --------------------------------------

TEST(ArithmeticTypeCheck, IntPlusIntIsInt) {
    const auto [prog, result] = analyze_ok("let x = 1 + 2;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Int);
}

TEST(ArithmeticTypeCheck, IntPlusDoubleIsDouble) {
    const auto [prog, result] = analyze_ok("let x = 1 + 2.5;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(ArithmeticTypeCheck, DoublePlusIntIsDouble) {
    const auto [prog, result] = analyze_ok("let x = 2.5 + 1;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(ArithmeticTypeCheck, DoublePlusDoubleIsDouble) {
    const auto [prog, result] = analyze_ok("let x = 1.5 + 2.5;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(ArithmeticTypeCheck, IntMinusIntIsInt) {
    const auto [prog, result] = analyze_ok("let x = 5 - 3;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Int);
}

TEST(ArithmeticTypeCheck, IntMultiplyDoubleIsDouble) {
    const auto [prog, result] = analyze_ok("let x = 2 * 1.5;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(ArithmeticTypeCheck, IntDivideIntIsDouble) {
    // Division always produces Double regardless of operand types.
    const auto [prog, result] = analyze_ok("let x = 6 / 2;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(ArithmeticTypeCheck, IntDivideDoubleIsDouble) {
    const auto [prog, result] = analyze_ok("let x = 6 / 2.0;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(ArithmeticTypeCheck, DoubleDivideIntIsDouble) {
    const auto [prog, result] = analyze_ok("let x = 6.0 / 2;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(ArithmeticTypeCheck, DoubleDivideDoubleIsDouble) {
    const auto [prog, result] = analyze_ok("let x = 6.0 / 2.0;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Double);
}

TEST(ArithmeticTypeCheck, IntModuloIntIsInt) {
    const auto [prog, result] = analyze_ok("let x = 7 % 3;");
    const auto& let = std::get<dsl::ast::LetStatement>(std::get<dsl::ast::StatementPtr>(prog->globals[0])->kind);
    const auto t = result.get_expression_type(*let.value);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->kind, TypeKind::Int);
}

// -- Error cases ---------------------------------------------------------------

TEST(ArithmeticTypeCheck, BoolPlusIntIsError) {
    const auto analyzed = analyze("let x = true + 1;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "numeric"));
}

TEST(ArithmeticTypeCheck, IntPlusBoolIsError) {
    const auto analyzed = analyze("let x = 1 + true;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "numeric"));
}

TEST(ArithmeticTypeCheck, NotePlusIntIsError) {
    const auto analyzed = analyze("let x = A4 + 1;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "numeric"));
}

TEST(ArithmeticTypeCheck, IntPlusNoteIsError) {
    const auto analyzed = analyze("let x = 1 + A4;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "numeric"));
}

TEST(ArithmeticTypeCheck, DoubleModuloIntIsError) {
    const auto analyzed = analyze("let x = 1.5 % 1;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "modulo"));
}

TEST(ArithmeticTypeCheck, IntModuloDoubleIsError) {
    const auto analyzed = analyze("let x = 3 % 1.5;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "modulo"));
}

TEST(ArithmeticTypeCheck, DoubleModuloDoubleIsError) {
    const auto analyzed = analyze("let x = 1.5 % 2.5;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "modulo"));
}

TEST(ArithmeticTypeCheck, ModuloWithKnownDoubleVariableIsError) {
    const auto analyzed = analyze("let d = 1.5; let x = d % 2;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "modulo"));
}
