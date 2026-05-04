#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Baseline: reads always OK -------------------------------------------------

TEST(ScopeAccess, TrackBodyCanReadGlobal) {
    const auto [prog, result] = analyze_ok(R"(
        let x = 5;
        track { play A4 :x; }
    )");
}

TEST(ScopeAccess, TrackBodyCanReadTrackLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 5;
            play A4 :x;
        }
    )");
}

TEST(ScopeAccess, VoiceBodyCanReadGlobal) {
    const auto [prog, result] = analyze_ok(R"(
        let x = 5;
        track {
            voice { play A4 :x; }
        }
    )");
}

TEST(ScopeAccess, VoiceBodyCanReadTrackLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 5;
            voice { play A4 :x; }
        }
    )");
}

TEST(ScopeAccess, PatternInTrackCanReadGlobal) {
    const auto [prog, result] = analyze_ok(R"(
        let x = 5;
        track {
            pattern p() { play A4 :x; }
            play p();
        }
    )");
}

TEST(ScopeAccess, PatternInTrackCanReadTrackLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 5;
            pattern p() { play A4 :x; }
            play p();
        }
    )");
}

TEST(ScopeAccess, PatternInVoiceCanReadVoiceLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            voice {
                let x = 5;
                pattern p() { play A4 :x; }
                play p();
            }
        }
    )");
}

TEST(ScopeAccess, PatternInVoiceCanReadTrackLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 5;
            voice {
                pattern p() { play A4 :x; }
                play p();
            }
        }
    )");
}

// -- Track body write-access ---------------------------------------------------

TEST(ScopeAccess, TrackBodyCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        track { x = 1; }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

TEST(ScopeAccess, TrackBodyCanWriteTrackLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 0;
            x = 5;
        }
    )");
}

// -- Pattern in global scope ---------------------------------------------------

TEST(ScopeAccess, GlobalPatternCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        pattern p() { x = 1; }
        track { play p(); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

// -- Pattern in track ---------------------------------------------------------

TEST(ScopeAccess, PatternInTrackCanWriteTrackLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 0;
            pattern p() { x = 5; }
            play p();
        }
    )");
}

TEST(ScopeAccess, PatternInTrackCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        track {
            pattern p() { x = 1; }
            play p();
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

// -- Voice body ----------------------------------------------------------------

TEST(ScopeAccess, VoiceBodyCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        track {
            voice { x = 1; }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

TEST(ScopeAccess, VoiceBodyCannotWriteTrackLocal) {
    const auto analyzed = analyze(R"(
        track {
            let x = 0;
            voice { x = 1; }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

TEST(ScopeAccess, VoiceBodyCanWriteVoiceLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            voice {
                let x = 0;
                x = 5;
            }
        }
    )");
}

// -- Pattern in voice ---------------------------------------------------------

TEST(ScopeAccess, PatternInVoiceCanWriteVoiceLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            voice {
                let x = 0;
                pattern p() { x = 5; }
                play p();
            }
        }
    )");
}

TEST(ScopeAccess, PatternInVoiceCannotWriteTrackLocal) {
    const auto analyzed = analyze(R"(
        track {
            let x = 0;
            voice {
                pattern p() { x = 1; }
                play p();
            }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

TEST(ScopeAccess, PatternInVoiceCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        track {
            voice {
                pattern p() { x = 1; }
                play p();
            }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

// -- Control flow in track -----------------------------------------------------

TEST(ScopeAccess, ForInTrackCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        track {
            for (let i = 0; i < 3; i = i + 1) { x = i; }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

TEST(ScopeAccess, ForInTrackCanWriteTrackLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 0;
            for (let i = 0; i < 3; i = i + 1) { x = i; }
        }
    )");
}

TEST(ScopeAccess, IfInTrackCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        track {
            if (true) { x = 1; }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

TEST(ScopeAccess, LoopInTrackCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        track {
            loop (3) { x = 1; }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

// -- Control flow in voice -----------------------------------------------------

TEST(ScopeAccess, ForInVoiceCannotWriteTrackLocal) {
    const auto analyzed = analyze(R"(
        track {
            let x = 0;
            voice {
                for (let i = 0; i < 3; i = i + 1) { x = i; }
            }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

TEST(ScopeAccess, ForInVoiceCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        track {
            voice {
                for (let i = 0; i < 3; i = i + 1) { x = i; }
            }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

TEST(ScopeAccess, ForInVoiceCanWriteVoiceLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            voice {
                let x = 0;
                for (let i = 0; i < 3; i = i + 1) { x = i; }
            }
        }
    )");
}

// -- Control flow in pattern in track -----------------------------------------

TEST(ScopeAccess, ForInPatternInTrackCanWriteTrackLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            let x = 0;
            pattern p() {
                for (let i = 0; i < 3; i = i + 1) { x = i; }
            }
            play p();
        }
    )");
}

TEST(ScopeAccess, ForInPatternInTrackCannotWriteGlobal) {
    const auto analyzed = analyze(R"(
        let x = 0;
        track {
            pattern p() {
                for (let i = 0; i < 3; i = i + 1) { x = i; }
            }
            play p();
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

// -- Control flow in pattern in voice -----------------------------------------

TEST(ScopeAccess, ForInPatternInVoiceCanWriteVoiceLocal) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            voice {
                let x = 0;
                pattern p() {
                    for (let i = 0; i < 3; i = i + 1) { x = i; }
                }
                play p();
            }
        }
    )");
}

TEST(ScopeAccess, ForInPatternInVoiceCannotWriteTrackLocal) {
    const auto analyzed = analyze(R"(
        track {
            let x = 0;
            voice {
                pattern p() {
                    for (let i = 0; i < 3; i = i + 1) { x = i; }
                }
                play p();
            }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "read-only"));
}

// -- Pattern parameter is not assignable --------------------------------------

TEST(ScopeAccess, PatternParameterIsImmutable) {
    const auto analyzed = analyze(R"(
        pattern p(n) {
            n = 5;
            play A4 :n;
        }
        track { play p(2); }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "'n'"));
}
