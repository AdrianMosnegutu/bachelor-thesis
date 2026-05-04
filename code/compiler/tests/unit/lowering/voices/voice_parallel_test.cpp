#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- Voice parallelism ---------------------------------------------------------

TEST(VoiceParallel, VoiceDoesNotAdvanceOuterCursor) {
    // Voice runs in parallel — after it finishes the outer cursor is where it
    // was before the voice.
    const auto ir = lower_ok("track { play A4; voice { play C5 :2; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    // A4 at 0, then B4 follows A4 in the outer cursor at beat 1
    EXPECT_EQ(notes_at(ir.tracks[0], 0.0), std::vector{69});
    EXPECT_EQ(notes_at(ir.tracks[0], 1.0), (std::vector{71, 72}));
}

TEST(VoiceParallel, VoiceStartsAtCurrentTrackCursor) {
    const auto ir = lower_ok("track { play A4 :4; voice { play C5; play E5; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_EQ(notes_at(ir.tracks[0], 0.0), (std::vector{69}));
    EXPECT_EQ(notes_at(ir.tracks[0], 4.0), (std::vector{72}));
    EXPECT_EQ(notes_at(ir.tracks[0], 5.0), (std::vector{76}));
}

TEST(VoiceParallel, TwoVoicesBothRunInParallel) {
    const auto ir = lower_ok(R"(
        track {
            voice { play A4; }
            voice { play C5; }
        }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(notes_at(ir.tracks[0], 0.0), (std::vector{69, 72}));
}

TEST(VoiceParallel, VoiceStartsAtCurrentOuterCursorPosition) {
    const auto ir = lower_ok("track { play A4 :2; voice { play B4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    // Voice starts where the outer cursor was (beat 2)
    const auto b4_event = std::find_if(ir.tracks[0].events.begin(), ir.tracks[0].events.end(), [](const auto& ev) {
        return ev.midi_note == 71;
    });
    ASSERT_NE(b4_event, ir.tracks[0].events.end());
    EXPECT_DOUBLE_EQ(b4_event->start_beat, 2.0);
}

TEST(VoiceParallel, VoiceLetBindingsDoNotLeakToOuterTrack) {
    // Bug guard: a voice's let x = 5 must not change x in the enclosing track.
    const auto ir = lower_ok(R"(
        track {
            let x = 1;
            voice { let x = 5; play A4 :x; }
            play B4 :x;
        }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    // A4 should have duration 5; B4 should have duration 1
    const auto* a4 = &ir.tracks[0].events[0];
    const auto* b4 = &ir.tracks[0].events[1];
    if (a4->midi_note == 71) std::swap(a4, b4);
    EXPECT_DOUBLE_EQ(a4->duration_beats, 5.0);
    EXPECT_DOUBLE_EQ(b4->duration_beats, 1.0);
}

TEST(VoiceParallel, VoiceRunsInParallelWithoutAdvancingOuterCursor) {
    const auto ir = lower_ok("track { play A4; voice { play C5 :2; } play B4; }");

    ASSERT_EQ(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];
    ASSERT_EQ(track.events.size(), 3u);
    EXPECT_EQ(notes_at(track, 0.0), std::vector{69});
    EXPECT_EQ(notes_at(track, 1.0), (std::vector{71, 72}));
}

TEST(VoiceParallel, VoiceLetBindingIndependentFromTrackBinding) {
    const auto ir = lower_ok(R"(
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
    if (events[0].midi_note == 71) {
        std::swap(voice_event, track_event);
    }
    EXPECT_EQ(voice_event->midi_note, 69);
    EXPECT_DOUBLE_EQ(voice_event->duration_beats, 5.0);
    EXPECT_EQ(track_event->midi_note, 71);
    EXPECT_DOUBLE_EQ(track_event->duration_beats, 1.0);
}
