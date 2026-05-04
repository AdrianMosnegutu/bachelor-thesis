#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

TEST(UnaryEval, NegationOfIntLiteral) {
    const auto ir = lower_ok("track { let x = -3 + 5; play A4 :x; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
}

TEST(UnaryEval, NegationOfFloatLiteral) {
    const auto ir = lower_ok("track { let x = -1.5 + 3.0; play A4 :x; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.5);
}

TEST(UnaryEval, NegationOfVariable) {
    const auto ir = lower_ok("track { let n = 2; let x = -n + 4; play A4 :x; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
}

TEST(UnaryEval, LogicalNotOfFalseTakesBranch) {
    const auto ir = lower_ok("track { if (!false) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);  // A4
}

TEST(UnaryEval, LogicalNotOfTrueSkipsBranch) {
    const auto ir = lower_ok("track { if (!true) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 71);  // B4 only
}

TEST(UnaryEval, LogicalNotOfVariableCondition) {
    const auto ir = lower_ok("track { let flag = false; if (!flag) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
}
