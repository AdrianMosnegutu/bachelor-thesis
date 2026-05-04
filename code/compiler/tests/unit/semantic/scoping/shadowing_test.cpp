#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Variable shadowing --------------------------------------------------------

TEST(Shadowing, TrackLocalShadowsGlobal) {
    // A let inside a track with the same name as a global let is valid —
    // the inner binding shadows the outer one within the track.
    const auto [prog, result] = analyze_ok(R"(
        let x = 1;
        track { let x = A4; }
    )");
}

TEST(Shadowing, TrackLocalShadowTakesInnerType) {
    // The inner declaration's type is what the inner binding resolves to.
    const auto [prog, result] = analyze_ok(R"(
        let x = 1;
        track {
            let x = A4;
            play x;
        }
    )");
}

TEST(Shadowing, GlobalStillAccessibleViaDifferentName) {
    // After shadowing, the original global is inaccessible under the same name,
    // but accessible under a different alias.
    const auto [prog, result] = analyze_ok(R"(
        let g = 1;
        track {
            let alias_g = g;
            let g = A4;
            play g;
        }
    )");
}

TEST(Shadowing, VoiceShadowsTrackLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 1;
            voice { let x = A4; play x; }
        }
    )");
}

TEST(Shadowing, ForLoopVarShadowsOuterVar) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let i = A4;
            for (let i = 0; i < 4; i = i + 1) { }
        }
    )");
}

TEST(Shadowing, PatternParamShadowsGlobal) {
    const auto [prog, result] = analyze_ok(R"(
        let n = A4;
        pattern p(n) { let x = n + 1; }
        track { play p(3); }
    )");
}
