#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- Beat positioning ----------------------------------------------------------

TEST(NoteCursor, FirstNoteStartsAtBeatZero) {
    const auto ir = lower_ok("track { play A4; }");
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
}

TEST(NoteCursor, SecondNoteStartsAfterFirstNoteDefaultDuration) {
    // Default duration is 1 beat.
    const auto ir = lower_ok("track { play A4; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
}

TEST(NoteCursor, ExplicitDurationAdvancesCursorByThatAmount) {
    const auto ir = lower_ok("track { play A4 :3; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 3.0);
}

TEST(NoteCursor, ExplicitDoubleDurationIsSupported) {
    const auto ir = lower_ok("track { play A4 :1.5; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.5);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.5);
}

TEST(NoteCursor, ThreeNotesHaveConsecutiveStartBeats) {
    const auto ir = lower_ok("track { play A4 :2; play B4 :3; play C5; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 5.0);
}
