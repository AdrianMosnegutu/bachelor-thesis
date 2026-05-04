#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Global scope visibility ----------------------------------------------------

TEST(VariableScoping, GlobalLetIsVisibleInsideTrack) {
    const auto [prog, result] = analyze_ok(R"(
        let x = 5;
        track { let y = x; }
    )");
}

TEST(VariableScoping, GlobalLetIsVisibleInsidePattern) {
    const auto [prog, result] = analyze_ok(R"(
        let x = 5;
        pattern p() { let y = x; }
    )");
}

TEST(VariableScoping, GlobalLetIsVisibleInsideVoice) {
    const auto [prog, result] = analyze_ok(R"(
        let x = 5;
        track { voice { let y = x; } }
    )");
}

// -- Track-local scope ---------------------------------------------------------

TEST(VariableScoping, TrackLocalLetNotVisibleInGlobalScope) {
    // A variable declared inside a track body should not bleed into global scope.
    // The only way to test this is via a pattern defined at global scope — it
    // must not see variables from any specific track.
    const auto analyzed = analyze(R"(
        pattern p() { let y = x; }
        track { let x = 5; }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(VariableScoping, TrackLocalLetNotVisibleInSiblingTrack) {
    const auto analyzed = analyze(R"(
        track { let x = 5; }
        track { let y = x; }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(VariableScoping, TrackLocalLetVisibleWithinSameTrack) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 5;
            let y = x;
        }
    )");
}

// -- Voice-local scope ---------------------------------------------------------

TEST(VariableScoping, VoiceLocalLetNotVisibleAfterVoiceInTrack) {
    // Variables declared inside a voice body must not be visible in the enclosing
    // track after the voice block ends.
    const auto analyzed = analyze(R"(
        track {
            voice { let x = 5; }
            let y = x;
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(VariableScoping, VoiceLocalLetVisibleWithinSameVoice) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            voice {
                let x = 5;
                let y = x;
            }
        }
    )");
}

TEST(VariableScoping, VoiceLocalLetNotVisibleInSiblingVoice) {
    const auto analyzed = analyze(R"(
        track {
            voice { let x = 5; }
            voice { let y = x; }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- For-loop variable scope ----------------------------------------------------

TEST(VariableScoping, ForLoopVariableVisibleInsideBody) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            for (let i = 0; i < 4; i = i + 1) { let y = i; }
        }
    )");
}

TEST(VariableScoping, ForLoopVariableNotVisibleAfterLoop) {
    const auto analyzed = analyze(R"(
        track {
            for (let i = 0; i < 4; i = i + 1) { }
            let y = i;
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- Sequential visibility -----------------------------------------------------

TEST(VariableScoping, LetNotVisibleBeforeItsDeclaration) {
    const auto analyzed = analyze(R"(
        track {
            let y = x;
            let x = 5;
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}
