#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- Arithmetic ----------------------------------------------------------------

TEST(BinaryEval, IntAddition) {
    const auto ir = lower_ok("track { let x = 2 + 3; play A4 :x; play B4; }");
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 5.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 5.0);
}

TEST(BinaryEval, IntSubtraction) {
    const auto ir = lower_ok("track { let x = 7 - 2; play A4 :x; play B4; }");
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 5.0);
}

TEST(BinaryEval, IntMultiplication) {
    const auto ir = lower_ok("track { let x = 2 * 3; play A4 :x; play B4; }");
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 6.0);
}

TEST(BinaryEval, DivisionProducesFloat) {
    const auto ir = lower_ok("track { let x = 5 / 2; play A4 :x; play B4; }");
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.5);
}

TEST(BinaryEval, IntPlusFloatPromotesToFloat) {
    const auto ir = lower_ok("track { let x = 1 + 0.5; play A4 :x; play B4; }");
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.5);
}

TEST(BinaryEval, DivisionByZeroEmitsLoweringDiagnostic) {
    const auto result = lower_with_diagnostics("track { let x = 1 / 0; play A4; }");
    EXPECT_TRUE(has_lowering_error(result.diagnostics.diagnostics(), "division by zero"));
}

// -- Comparison ----------------------------------------------------------------

TEST(BinaryEval, LessThanTrueEntersBranch) {
    const auto ir = lower_ok("track { if (1 < 2) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);  // A4
}

TEST(BinaryEval, LessThanFalseSkipsBranch) {
    const auto ir = lower_ok("track { if (2 < 1) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 71);  // B4 only
}

TEST(BinaryEval, LessThanOrEqualTrue) {
    const auto ir = lower_ok("track { if (2 <= 2) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(BinaryEval, GreaterThanTrue) {
    const auto ir = lower_ok("track { if (3 > 2) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(BinaryEval, GreaterThanOrEqualTrue) {
    const auto ir = lower_ok("track { if (2 >= 2) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(BinaryEval, EqualityTrue) {
    const auto ir = lower_ok("track { if (3 == 3) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(BinaryEval, EqualityFalse) {
    const auto ir = lower_ok("track { if (3 == 4) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
}

TEST(BinaryEval, InequalityTrue) {
    const auto ir = lower_ok("track { if (3 != 4) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
}

// -- Logical -------------------------------------------------------------------

TEST(BinaryEval, LogicalAndBothTrue) {
    const auto ir = lower_ok("track { if (true && true) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(BinaryEval, LogicalAndLeftFalseShortCircuits) {
    const auto ir = lower_ok("track { if (false && true) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
}

TEST(BinaryEval, LogicalOrLeftTrueShortCircuits) {
    const auto ir = lower_ok("track { if (true || false) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(BinaryEval, LogicalOrBothFalse) {
    const auto ir = lower_ok("track { if (false || false) { play A4; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
}
