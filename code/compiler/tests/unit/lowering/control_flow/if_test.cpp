#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- If statement --------------------------------------------------------------

TEST(IfStatement, TrueBranchTakenWhenConditionIsTrue) {
    const auto ir = lower_ok("track { if (true) { play A4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
}

TEST(IfStatement, TrueBranchNotTakenWhenConditionIsFalse) {
    const auto ir = lower_ok("track { if (false) { play A4; } }");
    EXPECT_TRUE(ir.tracks[0].events.empty());
}

TEST(IfStatement, FirstStatementUsedWhenNoBraces) {
    const auto ir = lower_ok("track { if (false) play A4; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 71);
}

TEST(IfStatement, FirstStatementUsedForElseWhenNoBraces) {
    const auto ir = lower_ok("track { if (true) play A4; else play B4; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);
}

TEST(IfStatement, NearestIfTakenIfChainedIfElseWithNoBraces) {
    const auto ir = lower_ok("track { if (true) if (false) play B4; else play C4; else play D4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 60);
}

TEST(IfStatement, ElseBranchTakenWhenConditionIsFalse) {
    const auto ir = lower_ok("track { if (false) { play A4; } else { play B4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 71);
}

TEST(IfStatement, ElseBranchNotTakenWhenConditionIsTrue) {
    const auto ir = lower_ok("track { if (true) { play A4; } else { play B4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
}

TEST(IfStatement, CursorAdvancesOnlyForTakenBranch) {
    // If-true takes A4 :2, then B4 follows at beat 2.
    const auto ir = lower_ok("track { if (true) { play A4 :2; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
}

TEST(IfStatement, CursorNotAdvancedWhenNoBranchTaken) {
    // Condition is false, no else — cursor stays at zero; B4 starts at 0.
    const auto ir = lower_ok("track { if (false) { play A4 :2; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
}

TEST(IfStatement, ConditionDrivenByVariable) {
    const auto ir = lower_ok(R"(
        let flag = true;
        track { if (flag) { play A4; } else { play B4; } }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
}

TEST(IfStatement, ConditionDrivenByExpression) {
    const auto ir = lower_ok(R"(
        track { if (5 < 10) { play A4; } else { play B4; } }
    )");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
}
