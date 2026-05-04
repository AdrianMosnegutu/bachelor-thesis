#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- Sequence item placement ---------------------------------------------------

TEST(SequenceCursor, SingleItemSequencePlacesNoteAtBeatZero) {
    const auto ir = lower_ok("track { play [A4]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
}

TEST(SequenceCursor, SequenceItemsArePlacedConsecutively) {
    const auto ir = lower_ok("track { play [A4, B4, C5]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 2.0);
}

TEST(SequenceCursor, SequenceItemsWithExplicitDurationsUseThoseDurations) {
    const auto ir = lower_ok("track { play [A4 :2, B4 :3, C5]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 5.0);
}

TEST(SequenceCursor, OuterCursorAdvancesByTotalSequenceDuration) {
    const auto ir = lower_ok("track { play [A4 :2, B4 :3]; play C5; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 5.0);
}

TEST(SequenceCursor, RestItemInSequenceProducesNoEventButAdvancesCursor) {
    const auto ir = lower_ok("track { play [rest :2, A4]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 2.0);
}

TEST(SequenceCursor, LeadingAndTrailingRestsContributeToTotalSpan) {
    const auto ir = lower_ok("track { play [rest :2, A4, rest :3]; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 6.0);
}

TEST(SequenceCursor, ChordItemInSequenceUsesMaxMemberDuration) {
    // Chord (A4 :3, C5 :1) has max duration 3; next item starts at beat 3.
    const auto ir = lower_ok("track { play [(A4 :3, C5 :1), B4]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_EQ(notes_at(ir.tracks[0], 0.0), (std::vector{69, 72}));
    EXPECT_EQ(notes_at(ir.tracks[0], 3.0), std::vector{71});
}

TEST(SequenceCursor, SequenceLeadingAndTrailingRestsAdvanceOuterCursor) {
    const auto ir = lower_ok("track { play [rest :2, A4, rest :3]; play B4; }");

    ASSERT_EQ(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];
    ASSERT_EQ(track.events.size(), 2u);
    EXPECT_EQ(notes_at(track, 2.0), std::vector{69});
    EXPECT_EQ(notes_at(track, 6.0), std::vector{71});
}
