#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

TEST(IdentifierEval, LetVariableResolvesToItsValue) {
    const auto ir = lower_ok("track { let x = 3; play A4 :x; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 3.0);
}

TEST(IdentifierEval, VariableUsedInArithmetic) {
    const auto ir = lower_ok("track { let a = 2; let b = a + 1; play A4 :b; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 3.0);
}

TEST(IdentifierEval, VariableUsedAsCondition) {
    const auto ir = lower_ok("track { let flag = true; if (flag) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);  // A4
}

TEST(IdentifierEval, ChainedVariableResolvesCorrectly) {
    const auto ir = lower_ok("track { let a = 2; let b = a; play A4 :b; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
}
