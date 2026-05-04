#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Basic pattern declarations ------------------------------------------------

TEST(PatternDeclaration, ParameterlessPatternIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        pattern melody() { play A4; }
        track { play melody(); }
    )");
}

TEST(PatternDeclaration, PatternWithOneParameterIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        pattern p(n) { let x = n + 1; }
        track { play p(3); }
    )");
}

TEST(PatternDeclaration, PatternWithMultipleParametersIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        pattern p(a, b) { let x = a + b; }
        track { play p(1, 2); }
    )");
}

// -- Pattern call arity --------------------------------------------------------

TEST(PatternDeclaration, CallingPatternWithWrongArityIsError) {
    const auto analyzed = analyze(R"(
        pattern p(a, b) { let x = a + b; }
        track { play p(1); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PatternDeclaration, CallingPatternWithExtraArgsIsError) {
    const auto analyzed = analyze(R"(
        pattern p() { play A4; }
        track { play p(1); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PatternDeclaration, CallingUndeclaredPatternIsError) {
    const auto analyzed = analyze(R"(
        track { play melody(); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- Pattern local let ---------------------------------------------------------

TEST(PatternDeclaration, PatternLocalLetIsVisible) {
    const auto [prog, result] = analyze_ok(R"(
        pattern p() {
            let x = 1;
            let y = x + 2;
        }
        track { play p(); }
    )");
}

// -- Pattern overloading -------------------------------------------------------

TEST(PatternDeclaration, PatternOverloadByArityIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        pattern p() { play A4; }
        pattern p(n) { play A4; }
        track {
            play p();
            play p(2);
        }
    )");
}

TEST(PatternDeclaration, DuplicatePatternArityIsError) {
    const auto analyzed = analyze(R"(
        pattern p(a) { play A4; }
        pattern p(b) { play B4; }
        track { play p(1); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}
