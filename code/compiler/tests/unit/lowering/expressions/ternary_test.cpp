#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

TEST(TernaryEval, TrueConditionSelectsThenBranch) {
    const auto ir = lower_ok("track { let x = true ? 2 : 5; play A4 :x; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
}

TEST(TernaryEval, FalseConditionSelectsElseBranch) {
    const auto ir = lower_ok("track { let x = false ? 2 : 5; play A4 :x; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 5.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 5.0);
}

TEST(TernaryEval, ConditionDerivedFromComparison) {
    const auto ir = lower_ok("track { let x = 3 > 1 ? 2 : 5; play A4 :x; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
}

TEST(TernaryEval, NestedTernaryInThenBranch) {
    // true ? (false ? 1 : 3) : 5  →  3
    const auto ir = lower_ok("track { let x = true ? (false ? 1 : 3) : 5; play A4 :x; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
}
