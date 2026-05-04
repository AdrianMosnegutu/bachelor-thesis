#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- For loop ------------------------------------------------------------------

TEST(ControlFlow, ForLoopWithAllPartsIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            for (let i = 0; i < 4; i = i + 1) { play A4; }
        }
    )");
}

TEST(ControlFlow, ForLoopWithBoolConditionIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            for (let i = 0; true; i = i + 1) { play A4; }
        }
    )");
}

TEST(ControlFlow, ForLoopOmittingInitIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let i = 0;
            for (; i < 4; i = i + 1) { play A4; }
        }
    )");
}

TEST(ControlFlow, ForLoopOmittingStepIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            for (let i = 0; i < 4;) { play A4; }
        }
    )");
}

TEST(ControlFlow, ForLoopWithNullConditionIsValid) {
    const auto analyzed = analyze_ok(R"(
        track { for (;;) { play A4; } }
    )");
}

TEST(ControlFlow, ForLoopWithOmittedInitAndStepButConditionPresentIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            for (; true;) { play A4; }
        }
    )");
}

TEST(ControlFlow, ForLoopConditionMustBeBool) {
    const auto analyzed = analyze(R"(
        track { for (let i = 0; i; i = i + 1) { play A4; } }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "condition"));
}

// -- If statement --------------------------------------------------------------

TEST(ControlFlow, IfWithBoolConditionIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track { if (true) { play A4; } }
    )");
}

TEST(ControlFlow, IfElseIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track { if (true) { play A4; } else { play B4; } }
    )");
}

TEST(ControlFlow, IfWithIntConditionIsError) {
    const auto analyzed = analyze(R"(
        track { if (1) { play A4; } }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "condition"));
}

TEST(ControlFlow, IfWithNoteConditionIsError) {
    const auto analyzed = analyze(R"(
        track { if (A4) { play A4; } }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "condition"));
}

// -- Loop statement ------------------------------------------------------------

TEST(ControlFlow, LoopWithPositiveIntCountIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track { loop (4) { play A4; } }
    )");
}

TEST(ControlFlow, LoopWithIdentifierCountIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        let n = 4;
        track { loop (n) { play A4; } }
    )");
}

TEST(ControlFlow, LoopWithNonIntCountIsError) {
    const auto analyzed = analyze(R"(
        track { loop (true) { play A4; } }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(ControlFlow, LoopWithDoubleCountIsError) {
    const auto analyzed = analyze(R"(
        track { loop (1.5) { play A4; } }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}
