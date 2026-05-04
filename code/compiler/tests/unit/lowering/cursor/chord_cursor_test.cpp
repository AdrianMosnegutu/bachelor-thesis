#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- Chord event placement -----------------------------------------------------

TEST(ChordCursor, TwoNoteChordBothStartAtSameBeat) {
    const auto ir = lower_ok("track { play (A4, C5); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(notes_at(ir.tracks[0], 0.0), (std::vector{69, 72}));
}

TEST(ChordCursor, ThreeNoteChordAllStartAtSameBeat) {
    const auto ir = lower_ok("track { play (A4, C5, E5); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_EQ(ir.tracks[0].events[1].start_beat, 0.0);
    EXPECT_EQ(ir.tracks[0].events[2].start_beat, 0.0);
}

TEST(ChordCursor, ChordDefaultDurationIsOneBeatPerMember) {
    const auto ir = lower_ok("track { play (A4, C5); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 1.0);
}

TEST(ChordCursor, ChordExplicitOuterDurationAppliedToAllMembers) {
    const auto ir = lower_ok("track { play (A4, C5) :3; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

TEST(ChordCursor, ChordMemberExplicitDurationsAreRespected) {
    const auto ir = lower_ok("track { play (A4 :3, C5 :1); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    const auto* a4 = &ir.tracks[0].events[0];
    const auto* c5 = &ir.tracks[0].events[1];
    if (a4->midi_note == 84) std::swap(a4, c5);
    EXPECT_DOUBLE_EQ(a4->duration_beats, 3.0);
    EXPECT_DOUBLE_EQ(c5->duration_beats, 1.0);
}

TEST(ChordCursor, OuterCursorAdvancesByMaxMemberDuration) {
    // (A4 :3, C5 :1) — max duration is 3; next note starts at beat 3.
    const auto ir = lower_ok("track { play (A4 :3, C5 :1); play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_EQ(notes_at(ir.tracks[0], 0.0), (std::vector{69, 72}));
    EXPECT_EQ(notes_at(ir.tracks[0], 3.0), std::vector{71});
}

TEST(ChordCursor, ChordWithUniformDefaultDurationsAdvancesCursorByOne) {
    const auto ir = lower_ok("track { play (A4, C5); play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_EQ(notes_at(ir.tracks[0], 1.0), std::vector{71});
}

TEST(ChordCursor, ChordDurationUsesMaximumMemberDuration) {
    const auto ir = lower_ok("track { play (A4 :3, C5 :1); play B4; }");

    ASSERT_EQ(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];
    ASSERT_EQ(track.events.size(), 3u);
    EXPECT_EQ(notes_at(track, 0.0), (std::vector{69, 72}));
    EXPECT_EQ(notes_at(track, 3.0), std::vector{71});
}
