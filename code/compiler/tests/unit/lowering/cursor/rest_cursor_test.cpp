#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- Rest placement ------------------------------------------------------------

TEST(RestCursor, RestProducesNoNoteEvent) {
    const auto ir = lower_ok("track { play rest; }");
    EXPECT_TRUE(ir.tracks[0].events.empty());
}

TEST(RestCursor, RestWithExplicitDurationProducesNoNoteEvent) {
    const auto ir = lower_ok("track { play rest :3; }");
    EXPECT_TRUE(ir.tracks[0].events.empty());
}

TEST(RestCursor, RestDefaultDurationAdvancesCursorByOneBeat) {
    const auto ir = lower_ok("track { play rest; play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 1.0);
}

TEST(RestCursor, RestExplicitDurationAdvancesCursorByThatAmount) {
    const auto ir = lower_ok("track { play rest :3; play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 3.0);
}

TEST(RestCursor, MultipleRestsAccumulateCursorOffset) {
    const auto ir = lower_ok("track { play rest :2; play rest :3; play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 5.0);
}

TEST(RestCursor, NoteRestNoteHaveCorrectPositions) {
    const auto ir = lower_ok("track { play A4; play rest :2; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 3.0);
}
