#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- Play from-offset mechanics ------------------------------------------------

TEST(FromOffset, PlayFromZeroStartsAtBeatZero) {
    const auto ir = lower_ok("track { play A4 from 0; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
}

TEST(FromOffset, PlayFromLiteralStartsAtThatBeat) {
    const auto ir = lower_ok("track { play A4 from 4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 4.0);
}

TEST(FromOffset, PlayFromExpressionIsEvaluated) {
    const auto ir = lower_ok("track { let start = 3; play A4 from start + 1; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 4.0);
}

TEST(FromOffset, PlayFromOffsetDoesNotAffectOuterCursor) {
    const auto ir = lower_ok(R"(
        track {
            play A4 from 2;
            play C5;
        }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(notes_at(ir.tracks[0], 0.0), std::vector{72});
    EXPECT_EQ(notes_at(ir.tracks[0], 2.0), std::vector{69});
}

TEST(FromOffset, NegativeFromIsError) {
    const auto ir = lower_with_diagnostics(R"(
        track {
            play A4 from -12;
            play C5;
        }
    )");

    EXPECT_TRUE(has_lowering_error(ir.diagnostics.diagnostics(), "negative"));
}

// -- Voice from-offset mechanics -----------------------------------------------

TEST(FromOffset, VoiceFromZeroStartsAtBeatZero) {
    const auto ir = lower_ok("track { voice from 0 { play A4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
}

TEST(FromOffset, VoiceFromLiteralStartsAtThatBeat) {
    const auto ir = lower_ok("track { voice from 4 { play A4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 4.0);
}

TEST(FromOffset, VoiceFromExpressionIsEvaluated) {
    const auto ir = lower_ok("track { let start = 3; voice from start + 1 { play A4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 4.0);
}

TEST(FromOffset, VoiceFromOffsetDoesNotAffectOuterCursor) {
    // A voice is parallel — it does not advance the enclosing track's cursor.
    // Events sorted by start_beat: A4(0), C5(1), B4(5).
    const auto ir = lower_ok(R"(
        track {
            play A4;
            voice from 5 { play B4; }
            play C5;
        }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_EQ(notes_at(ir.tracks[0], 0.0), std::vector{69});  // A4
    EXPECT_EQ(notes_at(ir.tracks[0], 1.0), std::vector{72});  // C5 follows A4, not the voice
    EXPECT_EQ(notes_at(ir.tracks[0], 5.0), std::vector{71});  // B4 from voice at beat 5
}

TEST(FromOffset, VoiceFromExpressionUsesEvaluatedOffset) {
    const auto ir = lower_ok("track { let start = 3; voice from start + 1 { play A4; } }");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 4.0);
}
