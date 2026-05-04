#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Happy flows ---------------------------------------------------------------

TEST(ChordTypeCheck, TwoNoteChordIsValid) { const auto [prog, result] = analyze_ok("track { play (A4, C5); }"); }

TEST(ChordTypeCheck, ThreeNoteChordIsValid) { const auto [prog, result] = analyze_ok("track { play (A4, C5, E5); }"); }

TEST(ChordTypeCheck, ChordWithExplicitMemberDurationsIsValid) {
    const auto [prog, result] = analyze_ok("track { play (A4 :3, C5 :1); }");
}

TEST(ChordTypeCheck, ChordAssignedToLetIsValid) { const auto [prog, result] = analyze_ok("let c = (A4, C5);"); }

TEST(ChordTypeCheck, ChordWithDurationsAssignedToLetIsValid) {
    const auto [prog, result] = analyze_ok("let c = (A4 :2, C5 :1);");
}

TEST(ChordTypeCheck, ChordUsedAsPlayTargetViaIdentifierIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        let c = (A4, C5);
        track { play c; }
    )");
}

// -- Error cases ---------------------------------------------------------------

TEST(ChordTypeCheck, IntMemberIsError) {
    const auto analyzed = analyze("track { play (42, A4); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "chord"));
}

TEST(ChordTypeCheck, BoolMemberIsError) {
    const auto analyzed = analyze("track { play (true, A4); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "chord"));
}

TEST(ChordTypeCheck, DoubleMemberIsError) {
    const auto analyzed = analyze("track { play (1.5, A4); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "chord"));
}

TEST(ChordTypeCheck, IdentifierOfIntTypeAsMemberIsError) {
    const auto analyzed = analyze("track { let n = 1; play (n, A4); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "chord"));
}

TEST(ChordTypeCheck, NonNumericMemberDurationIsError) {
    const auto analyzed = analyze("track { play (A4 :true, C5); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "duration"));
}

TEST(ChordTypeCheck, NoteAsDurationIsError) {
    const auto analyzed = analyze("track { play (A4 :B4, C5); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "duration"));
}
