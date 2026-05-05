#include <gtest/gtest.h>

#include <vector>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

TEST(MusicalEval, NoteLiteralProducesCorrectMidiNote) {
    const auto ir = lower_ok("track { play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);  // A4
}

TEST(MusicalEval, NoteLiteralHasDefaultDuration) {
    const auto ir = lower_ok("track { play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
}

TEST(MusicalEval, RestLiteralAdvancesCursorWithoutEvent) {
    const auto ir = lower_ok("track { play rest:2; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 71);  // B4
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 2.0);
}

TEST(MusicalEval, ChordProducesOneEventPerNote) {
    const auto ir = lower_ok("track { play (C4, E4, G4); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
}

TEST(MusicalEval, ChordNotesAllShareTheSameStartBeat) {
    const auto ir = lower_ok("track { play A4; play (C4, G4); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    // The chord starts after the A4 (at beat 1)
    for (std::size_t i = 1; i < ir.tracks[0].events.size(); ++i) {
        EXPECT_DOUBLE_EQ(ir.tracks[0].events[i].start_beat, 1.0);
    }
}

TEST(MusicalEval, SequenceLiteralEmitsNotesInOrder) {
    const auto ir = lower_ok("track { play [A4, B4]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);  // A4 at beat 0
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);  // B4 at beat 1
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
}

TEST(MusicalEval, SequenceLiteralCursorAdvancesAfterSequence) {
    const auto ir = lower_ok("track { play [A4, B4]; play C4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[2].start_beat, 2.0);
}
