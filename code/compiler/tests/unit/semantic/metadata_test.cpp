#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Tempo ---------------------------------------------------------------------

TEST(Metadata, TempoOfOneIsValid) { const auto [prog, result] = analyze_ok("tempo 1;"); }

TEST(Metadata, TempoOf1000IsValid) { const auto [prog, result] = analyze_ok("tempo 1000;"); }

TEST(Metadata, TempoOf120IsValid) { const auto [prog, result] = analyze_ok("tempo 120;"); }

TEST(Metadata, TempoOfZeroIsError) {
    const auto analyzed = analyze("tempo 0;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "tempo"));
}

TEST(Metadata, TempoGrammarOnlyAcceptsPositiveIntegerLiterals) {
    // The grammar restricts tempo to integer literals only, so negative values
    // cannot be expressed syntactically. Only zero and out-of-range positive
    // values need semantic validation.
    // This documents the grammar constraint.
    const auto analyzed = analyze("tempo 0;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "tempo"));
}

TEST(Metadata, TempoAbove1000IsError) {
    const auto analyzed = analyze("tempo 1001;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "tempo"));
}

// -- Time signature ------------------------------------------------------------

TEST(Metadata, TimeSignature4_4IsValid) { const auto [prog, result] = analyze_ok("signature 4/4;"); }

TEST(Metadata, TimeSignature3_4IsValid) { const auto [prog, result] = analyze_ok("signature 3/4;"); }

TEST(Metadata, TimeSignature6_8IsValid) { const auto [prog, result] = analyze_ok("signature 6/8;"); }

TEST(Metadata, TimeSignature1_1IsValid) { const auto [prog, result] = analyze_ok("signature 1/1;"); }

TEST(Metadata, TimeSignature32_32IsValid) { const auto [prog, result] = analyze_ok("signature 32/32;"); }

TEST(Metadata, TimeSignatureNumeratorZeroIsError) {
    const auto analyzed = analyze("signature 0/4;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "time signature"));
}

TEST(Metadata, TimeSignatureNumeratorAbove32IsError) {
    const auto analyzed = analyze("signature 33/4;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "time signature"));
}

TEST(Metadata, TimeSignatureDenominatorZeroIsError) {
    const auto analyzed = analyze("signature 4/0;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "time signature"));
}

TEST(Metadata, TimeSignatureDenominatorAbove32IsError) {
    const auto analyzed = analyze("signature 4/33;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "time signature"));
}

TEST(Metadata, TimeSignatureDenominatorMustBePowerOfTwo) {
    // Valid denominators: 1, 2, 4, 8, 16, 32. 3 is not a power of 2.
    const auto analyzed = analyze("signature 4/3;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "time signature"));
}

TEST(Metadata, TimeSignatureDenominator6IsError) {
    const auto analyzed = analyze("signature 6/6;");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
    EXPECT_TRUE(has_error(analyzed.diagnostics, "time signature"));
}
