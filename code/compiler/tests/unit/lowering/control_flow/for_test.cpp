#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- For loop ------------------------------------------------------------------

TEST(ForLoop, BasicCountingLoopProducesCorrectNumberOfEvents) {
    const auto ir = lower_ok("track { for (let i = 0; i < 4; i = i + 1) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 4u);
}

TEST(ForLoop, IterationVariableUsedAsDuration) {
    const auto ir = lower_ok("track { for (let i = 1; i <= 3; i = i + 1) { play A4 :i; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].duration_beats, 3.0);
}

TEST(ForLoop, CursorAccumulatesAcrossIterations) {
    const auto ir = lower_ok("track { for (let i = 1; i <= 3; i = i + 1) { play A4 :i; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    // Durations: 1, 2, 3 → start beats: 0, 1, 3
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 3.0);
}

TEST(ForLoop, IterationVariableCanBeUsedFromOutside) {
    const auto ir = lower_ok("track { let i = 1; for (; i <= 3; i = i + 1) { play A4 :i; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    // Durations: 1, 2, 3 → start beats: 0, 1, 3
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 3.0);
}

TEST(ForLoop, ConditionalInsideForEmitsCorrectSubset) {
    // Only even iterations: i = 0, 2 → 2 notes
    const auto ir = lower_ok(R"(
        track {
            for (let i = 0; i < 4; i = i + 1) {
                if (i % 2 == 0) { play A4; }
            }
        }
    )");
    EXPECT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(ForLoop, ForLoopWithStepGreaterThanOneSkipsValues) {
    // i = 0, 2 (step 2, condition i < 4) → 2 events
    const auto ir = lower_ok("track { for (let i = 0; i < 4; i = i + 2) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(ForLoop, ForLoopWithFalseConditionProducesNoEvents) {
    const auto ir = lower_ok("track { for (let i = 0; i < 0; i = i + 1) { play A4; } }");
    EXPECT_TRUE(ir.tracks[0].events.empty());
}

TEST(ForLoop, NoteAfterForLoopPositionedAfterLoopEnd) {
    const auto ir = lower_ok("track { for (let i = 0; i < 2; i = i + 1) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 2.0);
}

TEST(ForLoop, ForLoopIterationVariableHasCorrectValueEachIteration) {
    const auto ir = lower_ok(R"(
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
