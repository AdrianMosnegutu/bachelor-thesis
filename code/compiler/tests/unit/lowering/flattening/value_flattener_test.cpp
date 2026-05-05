#include "dsl/lowering/detail/value_flattener.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "dsl/common/ir/note_event.hpp"
#include "dsl/common/ir/values.hpp"

using namespace dsl::lowering::detail;
using namespace dsl::ir;

// -- flatten_note_value --------------------------------------------------------

TEST(FlattenNoteValue, DefaultDurationPassedThrough) {
    NoteValue note{60, 1.0, 100};
    NoteEvents events;
    flatten_note_value(note, 0.0, 1.0, events);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].midi_note, 60);
    EXPECT_DOUBLE_EQ(events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(events[0].duration_beats, 1.0);
    EXPECT_EQ(events[0].velocity, 100);
}

TEST(FlattenNoteValue, ExplicitDurationOverridesDefault) {
    // duration_beats=2.0 != 1.0 → override wins
    NoteValue note{60, 1.0, 100};
    NoteEvents events;
    flatten_note_value(note, 0.0, 2.0, events);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_DOUBLE_EQ(events[0].duration_beats, 2.0);
}

TEST(FlattenNoteValue, NoteOwnDurationPreservedWhenPassThroughIsDefault) {
    // note has explicit 3-beat duration; caller passes the default 1.0 → preserve 3.0
    NoteValue note{60, 3.0, 100};
    NoteEvents events;
    flatten_note_value(note, 0.0, 1.0, events);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_DOUBLE_EQ(events[0].duration_beats, 3.0);
}

TEST(FlattenNoteValue, ExplicitDurationOverridesNoteOwnDuration) {
    // both non-default; passed duration takes precedence
    NoteValue note{60, 3.0, 100};
    NoteEvents events;
    flatten_note_value(note, 0.0, 2.0, events);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_DOUBLE_EQ(events[0].duration_beats, 2.0);
}

TEST(FlattenNoteValue, StartBeatIsPreserved) {
    NoteValue note{72, 1.0, 80};
    NoteEvents events;
    flatten_note_value(note, 5.0, 1.0, events);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_DOUBLE_EQ(events[0].start_beat, 5.0);
}

TEST(FlattenNoteValue, VelocityIsPreserved) {
    NoteValue note{60, 1.0, 64};
    NoteEvents events;
    flatten_note_value(note, 0.0, 1.0, events);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].velocity, 64);
}

// -- flatten_chord_value -------------------------------------------------------

TEST(FlattenChordValue, TwoNotesEmitTwoEvents) {
    ChordValue chord{{NoteValue{60, 1.0, 100}, NoteValue{64, 1.0, 100}}, 1.0};
    NoteEvents events;
    flatten_chord_value(chord, 0.0, events);
    ASSERT_EQ(events.size(), 2u);
}

TEST(FlattenChordValue, AllNotesShareTheSameStartBeat) {
    ChordValue chord{{NoteValue{60, 1.0, 100}, NoteValue{64, 1.0, 100}, NoteValue{67, 1.0, 100}}, 1.0};
    NoteEvents events;
    flatten_chord_value(chord, 3.0, events);
    ASSERT_EQ(events.size(), 3u);
    for (const auto& ev : events) {
        EXPECT_DOUBLE_EQ(ev.start_beat, 3.0);
    }
}

TEST(FlattenChordValue, EachNotePreservesItsOwnDurationAndVelocity) {
    ChordValue chord{{NoteValue{60, 2.0, 80}, NoteValue{64, 3.0, 60}}, 2.0};
    NoteEvents events;
    flatten_chord_value(chord, 0.0, events);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].midi_note, 60);
    EXPECT_DOUBLE_EQ(events[0].duration_beats, 2.0);
    EXPECT_EQ(events[0].velocity, 80);
    EXPECT_EQ(events[1].midi_note, 64);
    EXPECT_DOUBLE_EQ(events[1].duration_beats, 3.0);
    EXPECT_EQ(events[1].velocity, 60);
}

// -- flatten_sequence_value ----------------------------------------------------

TEST(FlattenSequenceValue, SingleNoteStartsAtSequenceStart) {
    SequenceValue seq;
    seq.items.push_back(std::make_shared<Value>(NoteValue{60, 1.0, 100}));
    NoteEvents events;
    flatten_sequence_value(seq, 2.0, events);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_DOUBLE_EQ(events[0].start_beat, 2.0);
}

TEST(FlattenSequenceValue, SecondNoteStartsAfterFirstNoteDuration) {
    SequenceValue seq;
    seq.items.push_back(std::make_shared<Value>(NoteValue{60, 2.0, 100}));
    seq.items.push_back(std::make_shared<Value>(NoteValue{64, 1.0, 100}));
    NoteEvents events;
    flatten_sequence_value(seq, 0.0, events);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_DOUBLE_EQ(events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(events[1].start_beat, 2.0);
}

TEST(FlattenSequenceValue, RestAdvancesCursorWithoutEmittingEvent) {
    SequenceValue seq;
    seq.items.push_back(std::make_shared<Value>(RestValue{3.0}));
    seq.items.push_back(std::make_shared<Value>(NoteValue{60, 1.0, 100}));
    NoteEvents events;
    flatten_sequence_value(seq, 0.0, events);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_DOUBLE_EQ(events[0].start_beat, 3.0);
}

TEST(FlattenSequenceValue, MultipleRestsAccumulateCursor) {
    SequenceValue seq;
    seq.items.push_back(std::make_shared<Value>(RestValue{1.0}));
    seq.items.push_back(std::make_shared<Value>(RestValue{2.0}));
    seq.items.push_back(std::make_shared<Value>(NoteValue{60, 1.0, 100}));
    NoteEvents events;
    flatten_sequence_value(seq, 0.0, events);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_DOUBLE_EQ(events[0].start_beat, 3.0);
}

TEST(FlattenSequenceValue, MultipleChordsInsideASequenceMaintainTheirDurations) {
    SequenceValue seq;
    seq.items.push_back(std::make_shared<Value>(
        ChordValue{std::vector{NoteValue{60, 1.0, 100}, NoteValue{60, 1.0, 100}, NoteValue{60, 1.0, 100}}, 1.0}));
    seq.items.push_back(std::make_shared<Value>(
        ChordValue{std::vector{NoteValue{60, 1.0, 100}, NoteValue{60, 1.0, 100}, NoteValue{60, 1.0, 100}}, 2.0}));
    seq.items.push_back(std::make_shared<Value>(
        ChordValue{std::vector{NoteValue{60, 1.0, 100}, NoteValue{60, 3.0, 100}, NoteValue{60, 2.0, 100}}}));
    seq.items.push_back(std::make_shared<Value>(
        ChordValue{std::vector{NoteValue{60, 1.0, 100}, NoteValue{60, 1.0, 100}, NoteValue{60, 1.0, 100}}, 0.5}));
    seq.items.push_back(std::make_shared<Value>(NoteValue{60, 1.0, 100}));

    NoteEvents events;
    flatten_sequence_value(seq, 0.0, events);

    ASSERT_EQ(events.size(), 13u);
    EXPECT_DOUBLE_EQ(events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(events[3].start_beat, 1.0);
    EXPECT_DOUBLE_EQ(events[6].start_beat, 3.0);
    EXPECT_DOUBLE_EQ(events[9].start_beat, 6.0);
    EXPECT_DOUBLE_EQ(events[12].start_beat, 6.5);
}

// -- flatten_value dispatch ----------------------------------------------------

TEST(FlattenValue, NoteValueEmitsOneEvent) {
    const Value v{NoteValue{60, 1.0, 100}};
    const auto events = flatten_value(v, 0.0, 1.0);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].midi_note, 60);
}

TEST(FlattenValue, ChordValueEmitsMultipleEvents) {
    const Value v{ChordValue{{NoteValue{60, 1.0, 100}, NoteValue{64, 1.0, 100}}, 1.0}};
    const auto events = flatten_value(v, 0.0, 1.0);
    EXPECT_EQ(events.size(), 2u);
}

TEST(FlattenValue, SequenceValueEmitsEvents) {
    SequenceValue seq;
    seq.items.push_back(std::make_shared<Value>(NoteValue{60, 1.0, 100}));
    seq.items.push_back(std::make_shared<Value>(NoteValue{64, 1.0, 100}));
    const Value v{std::move(seq)};
    const auto events = flatten_value(v, 0.0, 1.0);
    EXPECT_EQ(events.size(), 2u);
}

TEST(FlattenValue, RestValueEmitsNoEvents) {
    const Value v{RestValue{2.0}};
    const auto events = flatten_value(v, 0.0, 2.0);
    EXPECT_TRUE(events.empty());
}

TEST(FlattenValue, IntValueEmitsNoEvents) {
    const Value v{42};
    const auto events = flatten_value(v, 0.0, 1.0);
    EXPECT_TRUE(events.empty());
}

TEST(FlattenValue, BoolValueEmitsNoEvents) {
    const Value v{true};
    const auto events = flatten_value(v, 0.0, 1.0);
    EXPECT_TRUE(events.empty());
}
