#include <gtest/gtest.h>

#include "support/semantic_test_utils.hpp"

using namespace dsl::testing::semantic;

// -- Instrument declaration ----------------------------------------------------

TEST(TrackDeclaration, TrackWithoutInstrumentIsValid) { const auto [prog, result] = analyze_ok("track { play A4; }"); }

TEST(TrackDeclaration, TrackUsingPianoIsValid) {
    const auto [prog, result] = analyze_ok("track using piano { play A4; }");
}

TEST(TrackDeclaration, TrackUsingGuitarIsValid) {
    const auto [prog, result] = analyze_ok("track using guitar { play A4; }");
}

TEST(TrackDeclaration, TrackUsingDrumsIsValid) {
    const auto [prog, result] = analyze_ok("track using drums { play kick; }");
}

TEST(TrackDeclaration, MultipleTracksAreValid) {
    const auto [prog, result] = analyze_ok(R"(
        track using piano { play A4; }
        track using guitar { play B4; }
    )");
}

// -- Instrument mixing invariant -----------------------------------------------

TEST(TrackDeclaration, MelodicNoteInDrumTrackIsError) {
    const auto analyzed = analyze("track using drums { play A4; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(TrackDeclaration, DrumNoteInMelodicTrackIsError) {
    const auto analyzed = analyze("track using piano { play kick; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(TrackDeclaration, DrumNoteInTrackWithoutInstrumentIsError) {
    // A track with no instrument declaration defaults to melodic — drum notes must be rejected.
    const auto analyzed = analyze("track { play kick; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(TrackDeclaration, DrumNoteInSequenceInMelodicTrackIsError) {
    const auto analyzed = analyze("track { play [A3, kick]; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(TrackDeclaration, MelodicNoteInSequenceDrumTrackIsError) {
    const auto analyzed = analyze("track using drums { play [A3, kick]; }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(TrackDeclaration, MelodicNoteInChordDrumTrackIsError) {
    const auto analyzed = analyze("track using drums { play (A3, B4); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(TrackDeclaration, MelodicNoteInPatternInDrumTrackIsError) {
    const auto analyzed = analyze("pattern foo() { play A4; } track using drums { play foo(); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

TEST(TrackDeclaration, DrumNoteInPatternInMelodicTrackIsError) {
    const auto analyzed = analyze("pattern foo() { play [kick, snare]; } track using violin { play foo(); }");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}

// -- Voice inside track --------------------------------------------------------

TEST(TrackDeclaration, TrackWithVoiceIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            voice { play A4; }
        }
    )");
}

TEST(TrackDeclaration, TrackWithMultipleVoicesIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track {
            voice { play A4; }
            voice { play C5; }
        }
    )");
}

TEST(TrackDeclaration, DrumVoiceInDrumTrackIsValid) {
    const auto [prog, result] = analyze_ok(R"(
        track using drums {
            voice { play kick; }
        }
    )");
}

TEST(TrackDeclaration, MelodicNoteInDrumVoiceIsError) {
    const auto analyzed = analyze(R"(
        track using drums {
            voice { play A4; }
        }
    )");
    EXPECT_TRUE(has_semantic_error(analyzed.diagnostics));
}
