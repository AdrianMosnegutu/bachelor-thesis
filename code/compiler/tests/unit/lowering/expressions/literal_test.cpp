#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

TEST(LiteralEval, IntLiteralUsedAsDuration) {
    const auto ir = lower_ok("track { play A4 :3; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 3.0);
}

TEST(LiteralEval, FloatLiteralUsedAsDuration) {
    const auto ir = lower_ok("track { play A4 :1.5; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.5);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.5);
}

TEST(LiteralEval, BoolTrueLiteralTakesIfBranch) {
    const auto ir = lower_ok("track { if (true) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);  // A4
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);  // B4
}

TEST(LiteralEval, BoolFalseLiteralSkipsIfBranch) {
    const auto ir = lower_ok("track { if (false) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 71);  // B4 only
}
