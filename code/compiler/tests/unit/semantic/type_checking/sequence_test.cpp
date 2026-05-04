#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Happy flows ---------------------------------------------------------------

TEST(SequenceTypeCheck, NoteItemsAreValid) { const auto [prog, result] = analyze_ok("track { play [A4, B4, C5]; }"); }

TEST(SequenceTypeCheck, NoteItemsWithExplicitDurationsAreValid) {
    const auto [prog, result] = analyze_ok("track { play [A4 :2, B4 :1, C5 :3]; }");
}

TEST(SequenceTypeCheck, RestItemIsValid) { const auto [prog, result] = analyze_ok("track { play [rest, A4]; }"); }

TEST(SequenceTypeCheck, RestItemWithDurationIsValid) {
    const auto [prog, result] = analyze_ok("track { play [rest :2, A4]; }");
}

TEST(SequenceTypeCheck, MixedNoteAndRestItemsAreValid) {
    const auto [prog, result] = analyze_ok("track { play [A4, rest :2, B4, rest]; }");
}

TEST(SequenceTypeCheck, ChordItemInSequenceIsValid) {
    const auto [prog, result] = analyze_ok("track { play [(A4, C5), B4]; }");
}

TEST(SequenceTypeCheck, ChordWithDurationInSequenceIsValid) {
    const auto [prog, result] = analyze_ok("track { play [(A4, C5) :2, B4]; }");
}

TEST(SequenceTypeCheck, DrumNoteItemInDrumSequenceIsValid) {
    const auto [prog, result] = analyze_ok("track using drums { play [kick, snare]; }");
}

TEST(SequenceTypeCheck, SequenceAssignedToLetIsValid) {
    const auto [prog, result] = analyze_ok("let s = [A4 :2, B4 :3];");
}

TEST(SequenceTypeCheck, SingleItemSequenceIsValid) { const auto [prog, result] = analyze_ok("track { play [A4]; }"); }

// -- Error cases ---------------------------------------------------------------

TEST(SequenceTypeCheck, IntItemIsError) {
    const auto analyzed = analyze("track { play [42, A4]; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(SequenceTypeCheck, BoolItemIsError) {
    const auto analyzed = analyze("track { play [true, A4]; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(SequenceTypeCheck, DoubleItemIsError) {
    const auto analyzed = analyze("track { play [1.5, A4]; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(SequenceTypeCheck, NonNumericItemDurationIsError) {
    const auto analyzed = analyze("track { play [A4 :true]; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "duration"));
}

TEST(SequenceTypeCheck, NoteDurationAsNoteIsError) {
    const auto analyzed = analyze("track { play [A4 :B4]; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "duration"));
}

TEST(SequenceTypeCheck, NestedSequenceIsError) {
    // Sequences cannot be nested: [[A4], B4] must be rejected
    const auto analyzed = analyze("track { play [[A4], B4]; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(SequenceTypeCheck, IdentifierOfIntTypeAsItemIsError) {
    const auto analyzed = analyze("track { let n = 1; play [n, A4]; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}
