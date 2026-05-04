#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Pattern visibility --------------------------------------------------------

TEST(PatternScoping, GlobalPatternVisibleInsideTrack) {
    const auto [prog, result] = analyze_ok(R"(
        pattern melody() { play A4; }
        track { play melody(); }
    )");
}

TEST(PatternScoping, GlobalPatternVisibleInsideVoice) {
    const auto [prog, result] = analyze_ok(R"(
        pattern melody() { play A4; }
        track { voice { play melody(); } }
    )");
}

TEST(PatternScoping, TrackLocalPatternVisibleWithinSameTrack) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            pattern local() { play A4; }
            play local();
        }
    )");
}

TEST(PatternScoping, TrackLocalPatternNotVisibleInSiblingTrack) {
    const auto analyzed = analyze(R"(
        track { pattern local() { play A4; } }
        track { play local(); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PatternScoping, TrackLocalPatternNotVisibleAtGlobalScope) {
    // Global patterns must not see patterns defined inside a specific track.
    const auto analyzed = analyze(R"(
        track { pattern local() { play A4; } }
        pattern outer() { play local(); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- Pattern overloading -------------------------------------------------------

TEST(PatternScoping, PatternsWithSameNameDifferentArityCoexist) {
    const auto [prog, result] = analyze_ok(R"(
        pattern p() { play A4; }
        pattern p(n) { play A4; }
        track {
            play p();
            play p(2);
        }
    )");
}

TEST(PatternScoping, DuplicatePatternSameArityIsError) {
    const auto analyzed = analyze(R"(
        pattern p() { play A4; }
        pattern p() { play B4; }
        track { play p(); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- Pattern parameters --------------------------------------------------------

TEST(PatternScoping, PatternParameterVisibleInsideBody) {
    const auto [prog, result] = analyze_ok(R"(
        pattern p(n) { let x = n; }
        track { play p(3); }
    )");
}

TEST(PatternScoping, PatternParameterNotVisibleOutsidePattern) {
    const auto analyzed = analyze(R"(
        pattern p(n) { play A4; }
        track { let x = n; }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PatternScoping, PatternParameterIsImmutable) {
    // Pattern parameters cannot be re-assigned (they behave as immutable bindings).
    const auto analyzed = analyze(R"(
        pattern p(n) { n = 5; }
        track { play p(3); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- Pattern nesting -----------------------------------------------------------

TEST(PatternScoping, PatternCallingAnotherPatternIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        pattern inner() { play A4; }
        pattern outer() { play inner(); }
        track { play outer(); }
    )");
}

TEST(PatternScoping, PatternCannotBeDefinedInsideAnotherPattern) {
    // The grammar uses `block` for pattern bodies, so nested pattern_def is
    // syntactically impossible — this must be rejected at parse time.
    const auto diags = analyze_expecting_parse_error(R"(
        pattern outer() {
            pattern inner() { play A4; }
        }
        track { play outer(); }
    )");
    EXPECT_TRUE(has_parse_error(diags));
}
