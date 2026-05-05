#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- Pattern call output --------------------------------------------------------

TEST(PatternCall, ParameterlessPatternEmitsExpectedEvents) {
    const auto ir = lower_ok(R"(
        pattern phrase() {
            play A4;
            play B4;
        }
        track { play phrase(); }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);
}

TEST(PatternCall, PatternEventsPositionedAtCallSiteCursor) {
    const auto ir = lower_ok(R"(
        pattern phrase() { play A4; play B4; }
        track { play C5; play phrase(); }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);  // C5
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);  // A4
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 2.0);  // B4
}

TEST(PatternCall, PatternWithParameterUsesPassedValue) {
    const auto ir = lower_ok(R"(
        pattern timed(d) { play A4 :d; }
        track { play timed(3); }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
}

TEST(PatternCall, OuterCursorAdvancedByPatternTotalDuration) {
    const auto ir = lower_ok(R"(
        pattern phrase() { play A4 :2; play B4 :3; }
        track { play phrase(); play C5; }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 5.0);
}

TEST(PatternCall, PatternCalledMultipleTimesSpacedCorrectly) {
    const auto ir = lower_ok(R"(
        pattern note() { play A4; }
        track { play note(); play note(); play note(); }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 2.0);
}

TEST(PatternCall, PatternArgumentsEvaluatedInCallerScope) {
    // Bug regression: arguments must be evaluated in caller scope, not callee.
    const auto ir = lower_ok(R"(
        pattern p(a, b) { play A4 :a; play B4 :b; }
        track {
            let n = 2;
            play p(n, n + 1);
        }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(PatternCall, PatternArgumentsResolveInCallerScopeBeforeCalleeBindings) {
    const auto ir = lower_ok(R"(
        pattern timed_pair(a, b) {
            play A4 :a;
            play B4 :b;
        }

        track {
            let n = 2;
            play timed_pair(n, n + 1);
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(PatternCall, PatternInternalRestsPreserveTemporalSpan) {
    const auto ir = lower_ok(R"(
        pattern r() {
            play A4;
            play rest :2;
            play B4;
        }

        track {
            play r();
            play C4;
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];
    ASSERT_EQ(track.events.size(), 3u);
    EXPECT_EQ(notes_at(track, 0.0), std::vector{69});
    EXPECT_EQ(notes_at(track, 3.0), std::vector{71});
    EXPECT_EQ(notes_at(track, 4.0), std::vector{60});
}

TEST(PatternCall, PatternParameterShadowsGlobalWithSameName) {
    const auto ir = lower_ok(R"(
        let n = 100;
        pattern p(n) { play A4 :n; }
        track { play p(2); }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
}

TEST(PatternCall, TwoPatternCallsWithIdenticalParameterNamesDontCrossContaminate) {
    const auto ir = lower_ok(R"(
        pattern first(n) { play A4 :n; }
        pattern second(n) { play B4 :n; }
        track {
            play first(1);
            play second(3);
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(PatternCall, OuterPatternParameterUsedAfterNestedPatternCall) {
    const auto ir = lower_ok(R"(
        pattern inner(x) { play A4 :x; }
        pattern outer(a, b) {
            play inner(a);
            play B4 :b;
        }
        track { play outer(1, 3); }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(PatternCall, TrackLocalPatternParameterUsedAfterNestedPatternCall) {
    const auto ir = lower_ok(R"(
        track {
            pattern inner(x) { play A4 :x; }
            pattern outer(a, b) {
                play inner(a);
                play B4 :b;
            }
            play outer(1, 3);
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(PatternCall, PatternWithForLoopCalledViaLoop) {
    const auto ir = lower_ok(R"(
        pattern phrase(n) {
            for (let i = 0; i < n; i = i + 1) {
                play A4;
            }
        }
        track { loop (2) { play phrase(3); } }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events.size(), 6u);
}

TEST(PatternCall, ThreeLevelNestedPatternCallsResolveCorrectly) {
    const auto ir = lower_ok(R"(
        pattern leaf(d) { play A4 :d; }
        pattern mid(d) { play leaf(d); play B4 :d; }
        pattern top(d) { play mid(d); play C4 :d; }
        track { play top(2); }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].duration_beats, 2.0);
}

// -- Pattern overloading by arity -----------------------------------------------

TEST(PatternCall, TwoArityOverloadsRouteToCorrectBody) {
    // 1-arg overload plays A4, 2-arg overload plays B4.
    // Calling each should emit exactly the note from that overload.
    const auto ir = lower_ok(R"(
        pattern foo(a) { play A4 :a; }
        pattern foo(a, b) { play B4 :b; }
        track {
            play foo(1);
            play foo(2, 3);
        }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);   // A4 from 1-arg overload
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);   // B4 from 2-arg overload
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(PatternCall, TrackLocalArityOverloadsRouteToCorrectBody) {
    const auto ir = lower_ok(R"(
        track {
            pattern note(a) { play A4 :a; }
            pattern note(a, b) { play B4 :b; }
            play note(2);
            play note(1, 4);
        }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 4.0);
}

TEST(PatternCall, ArityOverloadCallSiteCursorAdvancesCorrectly) {
    // Ensure cursor advances by the right amount regardless of which overload is chosen.
    const auto ir = lower_ok(R"(
        pattern p(d) { play A4 :d; }
        pattern p(d1, d2) { play A4 :d1; play B4 :d2; }
        track {
            play p(3);
            play C4;
        }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);  // A4 for 3 beats
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 3.0);      // C4 starts after 3-beat pattern
}

TEST(PatternCall, SameIdentifierPassedTwiceAsParamsResolveCorrecly) {
    const auto ir = lower_ok(R"(
        track {
            pattern foo1(a, b, c) {
                play a;
                play b;
                play c;
            }

            pattern foo2(a, b) {
                play foo1(a, a, b); 
            }

            play foo2(A4, B4);
        }
    )");

    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].midi_note, 71);
}
