#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- Loop statement ------------------------------------------------------------

TEST(LoopStatement, LoopThreeTimesProducesThreeEvents) {
    const auto ir = lower_ok("track { loop (3) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 3u);
}

TEST(LoopStatement, LoopOnceProducesOneEvent) {
    const auto ir = lower_ok("track { loop (1) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 1u);
}

TEST(LoopStatement, LoopZeroTimesProducesNoEvents) {
    const auto ir = lower_ok("track { loop (0) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 0u);
}

TEST(LoopStatement, LoopedNotesHaveConsecutiveStartBeats) {
    const auto ir = lower_ok("track { loop (3) { play A4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 2.0);
}

TEST(LoopStatement, LoopMultipleNotesPerIterationSpansCorrectly) {
    // Each iteration plays A4 :2 + B4 (default :1) = 3 beats.
    const auto ir = lower_ok("track { loop (2) { play A4 :2; play B4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 4u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 3.0);  // second iter A4
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[3].start_beat, 5.0);  // second iter B4
}

TEST(LoopStatement, NoteAfterLoopStartsAfterLoopEnd) {
    const auto ir = lower_ok("track { loop (2) { play A4 :2; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 4.0);
}

TEST(LoopStatement, LoopCountFromIdentifierIsRespected) {
    const auto ir = lower_ok("let n = 4; track { loop (n) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 4u);
}

TEST(LoopStatement, LoopCountFromExpressionIsRespected) {
    const auto ir = lower_ok("track { loop (2 * 2) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 4u);
}
