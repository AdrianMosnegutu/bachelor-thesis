#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "lowerer_test_support.hpp"

using dsl::testing::lowerer::lower;

namespace {

std::vector<int> notes_at(const dsl::ir::Track& track, const double beat) {
    std::vector<int> notes;
    for (const auto& ev : track.events) {
        if (std::abs(ev.start_beat - beat) < 1e-9) {
            notes.push_back(ev.midi_note);
        }
    }
    std::ranges::sort(notes);
    return notes;
}

}  // namespace

TEST(LoweringRegression, GlobalAndLocalBindingsDriveDurations) {
    const auto ir = lower(R"(
        let global_dur = 2;
        track {
            let local_dur = global_dur + 1;
            play A4 :global_dur;
            play B4 :local_dur;
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 83);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(LoweringRegression, PatternArgumentsResolveInCallerScopeBeforeCalleeBindings) {
    const auto ir = lower(R"(
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
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 83);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(LoweringRegression, PatternInternalRestsPreserveTemporalSpan) {
    const auto ir = lower(R"(
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
    EXPECT_EQ(notes_at(track, 0.0), std::vector{81});
    EXPECT_EQ(notes_at(track, 3.0), std::vector{83});
    EXPECT_EQ(notes_at(track, 4.0), std::vector{72});
}

TEST(LoweringRegression, SequenceLeadingAndTrailingRestsAdvanceOuterCursor) {
    const auto ir = lower("track { play [rest :2, A4, rest :3]; play B4; }");

    ASSERT_EQ(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];
    ASSERT_EQ(track.events.size(), 2u);
    EXPECT_EQ(notes_at(track, 2.0), std::vector{81});
    EXPECT_EQ(notes_at(track, 6.0), std::vector{83});
}

TEST(LoweringRegression, ChordDurationUsesMaximumMemberDuration) {
    const auto ir = lower("track { play (A4 :3, C5 :1); play B4; }");

    ASSERT_EQ(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];
    ASSERT_EQ(track.events.size(), 3u);
    EXPECT_EQ(notes_at(track, 0.0), (std::vector{81, 84}));
    EXPECT_EQ(notes_at(track, 3.0), std::vector{83});
}

TEST(LoweringRegression, ControlFlowUnrollsUsingEvaluatedBindings) {
    const auto ir = lower(R"(
        track {
            for (let i = 0; i < 4; i = i + 1) {
                if (i % 2 == 0) {
                    play A4;
                }
            }
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
}

TEST(LoweringRegression, VoiceRunsInParallelWithoutAdvancingOuterCursor) {
    const auto ir = lower("track { play A4; voice { play C5 :2; } play B4; }");

    ASSERT_EQ(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];
    ASSERT_EQ(track.events.size(), 3u);
    EXPECT_EQ(notes_at(track, 0.0), std::vector{81});
    EXPECT_EQ(notes_at(track, 1.0), (std::vector{83, 84}));
}

TEST(LoweringRegression, VoiceFromExpressionUsesEvaluatedOffset) {
    const auto ir = lower("track { let start = 3; voice from start + 1 { play A4; } }");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 4.0);
}

TEST(LoweringRegression, PatternParameterShadowsGlobalWithSameName) {
    const auto ir = lower(R"(
        let n = 100;
        pattern p(n) { play A4 :n; }
        track { play p(2); }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
}

TEST(LoweringRegression, TwoPatternCallsWithIdenticalParameterNamesDontCrossContaminate) {
    const auto ir = lower(R"(
        pattern first(n) { play A4 :n; }
        pattern second(n) { play B4 :n; }
        track {
            play first(1);
            play second(3);
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 83);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(LoweringRegression, ForLoopIterationVariableHasCorrectValueEachIteration) {
    const auto ir = lower(R"(
        track {
            for (let i = 1; i <= 4; i = i + 1) {
                play A4 :i;
            }
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 4u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].duration_beats, 3.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[3].duration_beats, 4.0);
}

TEST(LoweringRegression, VoiceLetBindingIndependentFromTrackBinding) {
    const auto ir = lower(R"(
        track {
            let x = 1;
            voice { let x = 5; play A4 :x; }
            play B4 :x;
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    const auto& events = ir.tracks[0].events;
    const auto* voice_event = &events[0];
    const auto* track_event = &events[1];
    if (events[0].midi_note == 83) {
        std::swap(voice_event, track_event);
    }
    EXPECT_EQ(voice_event->midi_note, 81);
    EXPECT_DOUBLE_EQ(voice_event->duration_beats, 5.0);
    EXPECT_EQ(track_event->midi_note, 83);
    EXPECT_DOUBLE_EQ(track_event->duration_beats, 1.0);
}
