#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Valid play targets --------------------------------------------------------

TEST(PlayStatement, PlayNoteIsValid) { const auto [prog, result] = analyze_ok("track { play A4; }"); }

TEST(PlayStatement, PlayNoteViaIdentifierIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        let n = A4;
        track { play n; }
    )");
}

TEST(PlayStatement, PlaySequenceIsValid) { const auto [prog, result] = analyze_ok("track { play [A4, B4]; }"); }

TEST(PlayStatement, PlaySequenceViaIdentifierIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        let s = [A4, B4];
        track { play s; }
    )");
}

TEST(PlayStatement, PlayChordIsValid) { const auto [prog, result] = analyze_ok("track { play (A4, C5); }"); }

TEST(PlayStatement, PlayChordViaIdentifierIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        let c = (A4, C5);
        track { play c; }
    )");
}

TEST(PlayStatement, PlayRestIsValid) { const auto [prog, result] = analyze_ok("track { play rest; }"); }

TEST(PlayStatement, PlayRestWithDurationIsValid) { const auto [prog, result] = analyze_ok("track { play rest :2; }"); }

TEST(PlayStatement, PlayNoteWithDurationIsValid) { const auto [prog, result] = analyze_ok("track { play A4 :2; }"); }

TEST(PlayStatement, PlayChordWithDurationIsValid) {
    const auto [prog, result] = analyze_ok("track { play (A4, C5) :3; }");
}

TEST(PlayStatement, PlayTernaryEvaluatingToNoteIsValid) {
    const auto [prog, result] = analyze_ok("track { play (true ? A4 : B4); }");
}

// -- Invalid play targets: raw literals ---------------------------------------

TEST(PlayStatement, PlayIntLiteralIsError) {
    const auto analyzed = analyze("track { play 42; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PlayStatement, PlayBoolLiteralIsError) {
    const auto analyzed = analyze("track { play true; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PlayStatement, PlayDoubleLiteralIsError) {
    const auto analyzed = analyze("track { play 1.5; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- Invalid play targets: parenthesized non-note expressions -----------------

TEST(PlayStatement, PlayParenthesizedIntIsError) {
    // (123) is not a chord — a chord requires two or more note members.
    // A parenthesized int expression is still an int and must be rejected.
    const auto analyzed = analyze("track { play (123); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PlayStatement, PlayParenthesizedBoolIsError) {
    const auto analyzed = analyze("track { play (true); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PlayStatement, PlayParenthesizedTernaryOfIntIsError) {
    const auto analyzed = analyze("track { play (true ? 1 : 2); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- Invalid play targets: identifiers resolving to wrong type ----------------

TEST(PlayStatement, PlayIdentifierOfIntTypeIsError) {
    const auto analyzed = analyze(R"(
        let n = 42;
        track { play n; }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PlayStatement, PlayIdentifierOfBoolTypeIsError) {
    const auto analyzed = analyze(R"(
        let b = true;
        track { play b; }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(PlayStatement, PlayIdentifierOfDoubleTypeIsError) {
    const auto analyzed = analyze(R"(
        let d = 1.5;
        track { play d; }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- Duration type invariants --------------------------------------------------

TEST(PlayStatement, NonNumericDurationOnNoteIsError) {
    const auto analyzed = analyze("track { play A4 :true; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "duration"));
}

TEST(PlayStatement, NoteAsDurationOnNoteIsError) {
    const auto analyzed = analyze("track { play A4 :B4; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "duration"));
}

TEST(PlayStatement, NonNumericDurationOnRestIsError) {
    const auto analyzed = analyze("track { play rest :true; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "duration"));
}

TEST(PlayStatement, NonNumericDurationOnChordIsError) {
    const auto analyzed = analyze("track { play (A4, C5) :true; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "duration"));
}
